#include "XPLM/XPLMPlugin.h"
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"
#include <OpenGL/gl.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include "XPLMDisplay.h"

#define GEOJSON_FILE_PATH "path to openair_bay_airspace.geojson"

// Airspace struct to hold parsed data
typedef struct {
    double coordinates[10000][2]; // Allow up to 10,000 vertices
    int vertex_count;
    double lower_altitude;
    double upper_altitude;
    int class_type;      // Numeric class mapping for legacy support
    char *class_value;   // A, B, C ...
    char *type_value;    // DANGER, RESTRICTED...
} Airspace;


// Airspace data container
#define MAX_AIRSPACES 3000
Airspace airspaces[MAX_AIRSPACES];
int airspace_count = 0;

// Wrapper func to convert latitude/longitude/altitude to OpenGL coordinates
void latLonToLocal(double lat, double lon, double alt, float *x, float *y, float *z) {
    double dx, dy, dz;
    XPLMWorldToLocal(lat, lon, alt, &dx, &dy, &dz);
    *x = (float)dx;
    *y = (float)dy;
    *z = (float)dz;
}

// Debug function to print vertex data
//void debugPrintVertexData(Airspace *airspace) {
//    char debugBuffer[512];
//    snprintf(debugBuffer, sizeof(debugBuffer), "Airspace with %d vertices:\n", airspace->vertex_count);
//    XPLMDebugString(debugBuffer);
//   for (int i = 0; i < airspace->vertex_count; i++) {
//        snprintf(debugBuffer, sizeof(debugBuffer), "Vertex %d: [%f, %f]\n", i,
//                airspace->coordinates[i][0], airspace->coordinates[i][1]);
//        XPLMDebugString(debugBuffer);
//   }
//}


// Class to numeric type for compatibility
int mapClassToType(const char *classValue) {
    if (!classValue) return -1; // Undefined
    if (strcmp(classValue, "A") == 0) return 1;
    if (strcmp(classValue, "B") == 0) return 2;
    if (strcmp(classValue, "C") == 0) return 3;
    if (strcmp(classValue, "D") == 0) return 4;
    if (strcmp(classValue, "E") == 0) return 5;
    if (strcmp(classValue, "UNCLASSIFIED") == 0) return 0; // Unclassified
    return -2; // Unknown
}

// Function to parse the GeoJSON file
void parseGeoJSON() {
    FILE *file = fopen(GEOJSON_FILE_PATH, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open GeoJSON file.\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(length + 1);
    fread(buffer, 1, length, file);
    fclose(file);
    buffer[length] = '\0';

    struct json_object *root = json_tokener_parse(buffer);
    free(buffer);
    if (!root) {
        fprintf(stderr, "Error: Failed to parse GeoJSON file.\n");
        return;
    }

    struct json_object *features;
    if (!json_object_object_get_ex(root, "features", &features)) {
        fprintf(stderr, "Error: No features found in GeoJSON.\n");
        json_object_put(root);
        return;
    }

    airspace_count=0;
    int feature_count=json_object_array_length(features);

    for (int i = 0; i < feature_count && airspace_count < MAX_AIRSPACES; i++) {
        struct json_object *feature=json_object_array_get_idx(features, i);
        struct json_object *geometry, *properties;

        if (!json_object_object_get_ex(feature, "geometry", &geometry) ||
            !json_object_object_get_ex(feature, "properties", &properties)) {
            continue;
        }

        struct json_object *geometryType, *coordinates;
        if (!json_object_object_get_ex(geometry, "type", &geometryType) ||
            !json_object_object_get_ex(geometry, "coordinates", &coordinates)) {
            continue;
        }

        if (strcmp(json_object_get_string(geometryType), "Polygon") != 0) {
            continue; // Skip non-polygon geometries
        }

        Airspace airspace = {0};
        struct json_object *outerRing=json_object_array_get_idx(coordinates, 0);
        int numPoints=json_object_array_length(outerRing);

        for (int j = 0; j<numPoints && j<10000; j++) {
            struct json_object *point =json_object_array_get_idx(outerRing, j);
            airspace.coordinates[j][0] = json_object_get_double(json_object_array_get_idx(point, 0));
            airspace.coordinates[j][1] = json_object_get_double(json_object_array_get_idx(point, 1));
        }

        // Ensure the polygon is closed
        if (numPoints > 0 && 
            (airspace.coordinates[0][0] != airspace.coordinates[numPoints - 1][0] ||
             airspace.coordinates[0][1] != airspace.coordinates[numPoints - 1][1])) {
            airspace.coordinates[numPoints][0] = airspace.coordinates[0][0];
            airspace.coordinates[numPoints][1] = airspace.coordinates[0][1];
            numPoints++;
        }

        airspace.vertex_count = numPoints;

        struct json_object *lowerCeiling, *upperCeiling;
        if (json_object_object_get_ex(properties, "lowerCeiling", &lowerCeiling) &&
            json_object_object_get_ex(properties, "upperCeiling", &upperCeiling)) {
            airspace.lower_altitude = json_object_get_double(json_object_object_get(lowerCeiling, "value"))*0.3048;
            airspace.upper_altitude = json_object_get_double(json_object_object_get(upperCeiling, "value"))*0.3048;
        } else {
            airspace.lower_altitude = 0; // Default if missing value
            airspace.upper_altitude = 0;
        }

        struct json_object *classField, *typeField;
        const char *classValue = NULL;
        const char *typeValue = NULL;

        if (json_object_object_get_ex(properties, "class", &classField)) {
            classValue = json_object_get_string(classField);
        }
        if (json_object_object_get_ex(properties, "type", &typeField)) {
            typeValue = json_object_get_string(typeField);
        }

        // Store values in airspace cont
        airspace.class_type = mapClassToType(classValue);
        airspace.class_value = classValue ? strdup(classValue) : NULL;
        airspace.type_value = typeValue ? strdup(typeValue) : NULL;

        airspaces[airspace_count++] = airspace;
    }

    json_object_put(root);
    fprintf(stdout, "Parsed %d airspaces from GeoJSON.\n", airspace_count);
}


// Function to draw a polygon as a wireframe
void drawPolygonWireframe(Airspace *airspace, float altitude, float r, float g, float b) {
    glColor3f(r, g, b); 

    glBegin(GL_LINE_LOOP); 
    for (int i = 0; i < airspace->vertex_count; i++) {
        float x, y, z;
        latLonToLocal(airspace->coordinates[i][1], airspace->coordinates[i][0], altitude, &x, &y, &z);
        glVertex3f(x, y, z); 
    }
    glEnd();
}

// Airspace color based on class & type
void getAirspaceColor(const char *class_value, const char *type_value, float *r, float *g, float *b) {
    // Check for special use airspace based on the "type" field
    if (type_value && (
            strcmp(type_value, "RESTRICTED") == 0 ||
            strcmp(type_value, "PROHIBITED") == 0 ||
            strcmp(type_value, "DANGER") == 0)) {
        *r = 0.7f; *g = 0.3f; *b = 0.3f; // Deeper Red for Special Use
        return;
    }

    // Color based on "class" field
    if (class_value) {
        if (strcmp(class_value, "A") == 0) {
            *r = 0.5f; *g = 0.2f; *b = 0.2f; //Red for Class A, not rendered 
        } else if (strcmp(class_value, "B") == 0) {
            *r = 0.2f; *g = 0.3f; *b = 0.8f; // Royal Blue for Class B
        } else if (strcmp(class_value, "C") == 0) {
            *r = 0.7f; *g = 0.3f; *b = 0.7f; //  Magenta for Class C
        } else if (strcmp(class_value, "D") == 0) {
            *r = 0.5f; *g = 0.5f; *b = 0.8f; //  Light Blue for Class D
        } else if (strcmp(class_value, "E") == 0) {
            *r = 0.3f; *g = 0.6f; *b = 0.3f; // Green for Class E
        } else if (strcmp(class_value, "UNCLASSIFIED") == 0) {
            *r = 0.7f; *g = 0.7f; *b = 0.7f; // Light Gray for Unclassified 
        } else {
            *r = 0.5f; *g = 0.4f; *b = 0.3f; // Brownish Gray for Unknown
        }
    } else {
        *r = 0.2f; *g = 0.2f; *b = 0.2f; // Dark Gray for Undefined
    }
}


// Function to draw a filled polygon
void drawPolygonFilled(Airspace *airspace, float altitude, float r, float g, float b, float alpha) {
    glColor4f(r, g, b, alpha); 
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < airspace->vertex_count; i++) {
        float x, y, z;
        latLonToLocal(airspace->coordinates[i][1], airspace->coordinates[i][0], altitude, &x, &y, &z);
        glVertex3f(x, y, z); // Draw each vertex
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    for (int i = airspace->vertex_count - 1; i >= 0; i--) {
        float x, y, z;
        latLonToLocal(airspace->coordinates[i][1], airspace->coordinates[i][0], altitude, &x, &y, &z);
        glVertex3f(x, y, z); // Draw each vertex
    }
    glEnd();
}

// Function to draw filled vertical walls
void drawVerticalWalls(Airspace *airspace, float r, float g, float b, float alpha) {
    glColor4f(r, g, b, alpha); 
    glBegin(GL_QUADS);
    for (int i = 0; i < airspace->vertex_count - 1; i++) {
        float x1, y1, z1, x2, y2, z2;
        latLonToLocal(airspace->coordinates[i][1], airspace->coordinates[i][0], airspace->lower_altitude, &x1, &y1, &z1);
        latLonToLocal(airspace->coordinates[i + 1][1], airspace->coordinates[i + 1][0], airspace->lower_altitude, &x2, &y2, &z2);
        glVertex3f(x1, y1, z1);
        glVertex3f(x2, y2, z2);

        latLonToLocal(airspace->coordinates[i + 1][1], airspace->coordinates[i + 1][0], airspace->upper_altitude, &x2, &y2, &z2);
        latLonToLocal(airspace->coordinates[i][1], airspace->coordinates[i][0], airspace->upper_altitude, &x1, &y1, &z1);
        glVertex3f(x2, y2, z2);
        glVertex3f(x1, y1, z1);
    }
    glEnd();
}


void drawAirspace(Airspace *airspace) {
    float r, g, b;

    // Pass both class & type to getAirspaceColor
    getAirspaceColor(airspace->class_value, airspace->type_value, &r, &g, &b);

    float alpha = 0.25f; // Transparency level for filled polygons

    // Draw filled bottom face
    drawPolygonFilled(airspace, airspace->lower_altitude, r, g, b, alpha);

    // Draw filled top face
    drawPolygonFilled(airspace, airspace->upper_altitude, r, g, b, alpha);

    // Draw filled vertical walls
    drawVerticalWalls(airspace, r, g, b, alpha);

    // Draw edge contours(outlines)
    glColor3f(r, g, b); // Solid color for outlines
    drawPolygonWireframe(airspace, airspace->lower_altitude, r, g, b); // Bottom edge
    drawPolygonWireframe(airspace, airspace->upper_altitude, r, g, b); // Top edge

    glBegin(GL_LINES);
    for (int i = 0; i < airspace->vertex_count; i++) {
        float x1, y1, z1, x2, y2, z2;
        latLonToLocal(airspace->coordinates[i][1], airspace->coordinates[i][0], airspace->lower_altitude, &x1, &y1, &z1);
        latLonToLocal(airspace->coordinates[i][1], airspace->coordinates[i][0], airspace->upper_altitude, &x2, &y2, &z2);
        glVertex3f(x1, y1, z1);
        glVertex3f(x2, y2, z2);
    }
    glEnd();
}


// Callback for rendering
int drawAirspacesCallback(XPLMDrawingPhase phase, int is_before, void *refcon) {
    if (phase == xplm_Phase_Objects && !is_before) {
        
        glEnable(GL_BLEND); // Enable OPGL blending for transparency
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        for (int i = 0; i < airspace_count; i++) {
            drawAirspace(&airspaces[i]);
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND); // Disable blending after drawing
    }
    return 1;
}


// Plugin initialization
PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    strncpy(outName, "Airspace Visualizer", 255);
    strncpy(outSig, "example.airspace_visualizer", 255);
    strncpy(outDesc, "Visualizes airspaces from GeoJSON data.", 255);

    parseGeoJSON();
    XPLMRegisterDrawCallback(drawAirspacesCallback, xplm_Phase_Objects, 0, NULL);
    XPLMDebugString("Airspace Visualizer Plugin initialized.\n");
    return 1;
}

PLUGIN_API void XPluginStop(void) {
    XPLMUnregisterDrawCallback(drawAirspacesCallback, xplm_Phase_Objects, 0, NULL);
    XPLMDebugString("Airspace Visualizer Plugin stopped.\n");
}
PLUGIN_API void XPluginEnable(void) {}
PLUGIN_API void XPluginDisable(void) {}
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void *inParam) {}
