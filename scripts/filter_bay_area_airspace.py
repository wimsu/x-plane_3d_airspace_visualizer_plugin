
import geojson
from shapely.geometry import shape, Polygon

# Define bounding box coordinates
BOUNDING_BOX = {
    "min_lat": 36.742498,
    "max_lat": 37.989780,
    "min_lon": -122.576065,
    "max_lon": -121.201421,
}

# Load the full US airspace GeoJSON file
INPUT_FILE = "path to openair_us_airspace.geojson"  
OUTPUT_FILE = "path to openair_bay_airspace.geojson"

def is_within_bounding_box(feature_geometry, bounding_box):
    """Check if the feature is within the bounding box."""
    feature_shape = shape(feature_geometry)
    bbox_polygon = Polygon([
        (bounding_box["min_lon"], bounding_box["min_lat"]),
        (bounding_box["max_lon"], bounding_box["min_lat"]),
        (bounding_box["max_lon"], bounding_box["max_lat"]),
        (bounding_box["min_lon"], bounding_box["max_lat"]),
        (bounding_box["min_lon"], bounding_box["min_lat"]),
    ])
    return feature_shape.intersects(bbox_polygon)

# Load the GeoJSON data
with open(INPUT_FILE, "r") as file:
    data = geojson.load(file)

# Filter the features based on the bounding box
filtered_features = [
    feature for feature in data["features"]
    if is_within_bounding_box(feature["geometry"], BOUNDING_BOX)
]

# Create a new GeoJSON object for the filtered features
filtered_geojson = geojson.FeatureCollection(filtered_features)

# Save the filtered data to a new GeoJSON file
with open(OUTPUT_FILE, "w") as file:
    geojson.dump(filtered_geojson, file)

print(f"Filtered GeoJSON saved to {OUTPUT_FILE}")

