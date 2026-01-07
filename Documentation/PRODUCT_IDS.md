# Orbitals Plugin Product IDs

These are the unique product IDs for each plugin in the NNAudio site product library.

## Product IDs

| Plugin | Product ID | Product Name (for site) |
|--------|------------|-------------------------|
| Tidal | 200001 | Tidal |
| Apogee | 200002 | Apogee |
| Kepler | 200003 | Kepler |
| Lagrange | 200004 | Lagrange |
| Eclipse | 200005 | Eclipse |
| Ion | 200006 | Ion |
| Perihelion | 200007 | Perihelion |
| Retrograde | 200008 | Retrograde |

## Usage

These product IDs are used in:
- License file validation (comma-separated list in license.dat)
- Authentication checks in each plugin's `checkAuthorization()` method
- Product identification in the NNAudio site product library
