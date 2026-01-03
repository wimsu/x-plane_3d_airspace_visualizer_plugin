# X-Plane Airspace Visualizer Plugin

A flight simulator plugin for visualization of 3-dimensional controlled airspaces and air traffic management research. This plugin integrates real-world geographic airspace data with OpenGL rendering techniques within the X-Plane 11/12 simulation environment, providing accurate 3D visualization of FAA-classified airspace networks for research in Advanced Air Mobility (AAM) and Urban Air Mobility (UAM).

## Features

- 3D Airspace Rendering
- FAA Airspace Classification

- **Platform Support**: Compatible with X-Plane 11 and 12 (x86_64)

## Requirements

- **X-Plane 11 or 12** (x86_64)
- **X-Plane SDK 2.1.3 or 4.1.1** (SDK is in directory: `SDK/` or `SDK 411/`)
- **clang++** (Xcode Command Line Tools)
- **json-c library** (`brew install json-c`)

**Note:** This plugin is compiled for x86_64 architecture, which works on Apple Silicon Macs (M1/M2/M3) via Rosetta 2 when X-Plane is running in x86_64 mode. This plugin was tested on Apple M3 Pro 2023

## Building

### Quick Build

```bash
./build.sh
```

### Manual Build

```bash
mkdir -p output
clang++ -arch x86_64 -std=c++17 -fPIC -shared -o output/airspace_visualizer_mac.xpl \
    -I./SDK/CHeaders \
    -I./SDK/CHeaders/XPLM \
    -I/usr/local/include/json-c \
    -F./SDK/Libraries/Mac \
    -L/usr/local/lib \
    -ljson-c \
    -DAPL=1 -DGL_SILENCE_DEPRECATION -DXPLM303 \
    -framework XPLM \
    -framework XPWidgets \
    -framework OpenGL \
    src/bayairspace_final.cpp
```

**Note:** If json-c is installed via Homebrew on Apple Silicon, you may need to add:
- `-I/opt/homebrew/include/json-c` (for headers)
- `-L/opt/homebrew/lib` (for libraries)

## Installation

1. Build the plugin (see Building section above)
2. Copy `output/airspace_visualizer_mac.xpl` to: `X-Plane/Resources/plugins/airspace_visualizer_plugin/airspace_visualizer_mac.xpl`
3. Update the GeoJSON file path in `src/bayairspace_final.cpp` (line 13) or place your GeoJSON file at the specified path
4. Enable plugin in X-Plane: **Plugin Admin → Enable "Airspace Visualizer"**

## Data Sources

### OpenAIP Integration

The plugin uses airspace data from [OpenAIP](https://www.openaip.net/), an open-source platform providing detailed aeronautical information including airspace boundaries. OpenAIP provides data in OpenAIR format, which is converted to GeoJSON for integration with the plugin.

### Converting OpenAIR Format to GeoJSON

GeoJSON files are generated from OpenAIR format using the included OpenAIP parser tool. You'll need the source OpenAIR file (e.g., `us_asp_extended.txt` from OpenAIP or other sources):

```bash
cd openaip-openair-parser
node cli.js -f /path/to/us_asp_extended.txt \
    -o ../data/openair_us_airspace.geojson --extended-format
```

The parser processes OpenAIR extended format files and outputs standard GeoJSON FeatureCollection format with polygon geometries and airspace properties (class, type, upperCeiling, lowerCeiling).

### Filtering to Specific Regions

Filter large GeoJSON files to specific geographic bounding boxes (currently set to the bay area) using the included Python script:

```bash
python3 scripts/filter_bay_area_airspace.py
```

**Requirements:** `pip install geojson shapely`

Edit the bounding box coordinates in the script to extract airspaces for other regios of interest.

### Airspace Limits

- Maximum airspaces: 3,000 (defined by `MAX_AIRSPACES` for performance)
- Maximum vertices per polygon: 10,000 (defined in `Airspace` struct for performance)

### Color Scheme

Airspaces are color-coded following FAA guidelines:
- **Class A**: Muted Red (RGB 0.5, 0.2, 0.2) - High-altitude controlled airspace
- **Class B**: Royal Blue (RGB 0.2, 0.3, 0.8) - Busiest airspace around major airports
- **Class C**: Magenta (RGB 0.7, 0.3, 0.7) - Mid-level controlled airspace
- **Class D**: Light Blue (RGB 0.5, 0.5, 0.8) - Lower-level control zones
- **Class E**: Green (RGB 0.3, 0.6, 0.3) - Controlled airspace filling gaps
- **UNCLASSIFIED**: Light Gray (RGB 0.7, 0.7, 0.7)
- **RESTRICTED/PROHIBITED/DANGER**: Deeper Red (RGB 0.7, 0.3, 0.3) - Special Use Airspaces

## Research Context

This plugin was developed for research in Advanced Air Mobility (AAM) and air traffic management visualization. It enables rapid prototyping and validation of novel airspace concepts of operations (ConOps) within a realistic flight simulation environment, supporting research in:

- Clean-slate airspace redesign approaches
- Urban Air Mobility (UAM) corridor visualization
- Air traffic density stress testing
- Controller-pilot evaluation studies

## Citation

If you use this work or find it helpful, we will be happy if you cite us! : (bibtex below)

@inproceedings{su2025flight,
  title={A Flight Simulator Software for Visualization of 3-Dimensional Airspaces and Air Traffic Management},
  author={Su, William and Kam, Jordan and Bulusu, Dr Vishwanath},
  booktitle={2025 Regional Student Conferences},
  pages={97940},
  year={2025}
}

## Acknowledgments

- Uses [OpenAIR Parser](https://github.com/openAIP/openaip-openair-parser) for OpenAIR to GeoJSON conversion
- Built with X-Plane SDK 2.1.3 (XPLM303)
- Airspace data sourced from [OpenAIP](https://www.openaip.net/)



