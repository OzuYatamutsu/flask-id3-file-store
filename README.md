# YTFS filestore server
This is the backend file store of YTFS.
 * The interface is powered by Python/Flask, and the metadata database is powered by MySQL.
 * Files are stored on the filesystem as normal, in a specified data directory.
 * All queries are done through a RESTful web interface (as detailed below).
 * Metadata is served in JSON format.

# Configuration
See `config.json` for server/database configuration variables.

# Installation
Run `install.sh`. This requires root system and database credentials (if `mysql-server` is already installed).

# API
## `GET` to `/ls`
**Parameters**: _None._

Returns a recursive listing of all files on the filesystem.<br />
Listings are done with these hierarchies:
```
"albums" -> (album) -> (path, title, track)
"decades" -> (decade) -> (album) -> (path, title, track)
```

### Example output
```
{
  "albums": {
    "The Black and White Album": [
      {
        "filename": "The_Hives_-_Hey_Little_World.mp3",
        "filesize": 8116224,
        "title": "Hey Little World",
        "track": 5
      },
      {
        "filename": "The_Hives_-_A_Stroll_Through_Hive_Manor_Co.mp3",
        "filesize": 6367232,
        "title": "A Stroll Through Hive Manor Corridors",
        "track": 6
      }    
    ],
    "Tyrannosaurus Hives": [
      {
        "filename": "The_Hives_-_Two-Timing_Touch_And_Broken_Bo.mp3",
        "filesize": 2908160,
        "title": "Two-Timing Touch And Broken Bones",
        "track": 2
      },
      {
        "filename": "The_Hives_-_Diabolic_Scheme.mp3",
        "filesize": 4341839,
        "title": "Diabolic Scheme",
        "track": 8
      }
    ]
  },
  "decades": {
    "2000": {
      "The Black and White Album": [
        {
          "filename": "The_Hives_-_Hey_Little_World.mp3",
          "filesize": 8116224,
          "title": "Hey Little World",
          "track": 5
        },
        {
          "filename": "The_Hives_-_A_Stroll_Through_Hive_Manor_Co.mp3",
          "filesize": 6367232,
          "title": "A Stroll Through Hive Manor Corridors",
          "track": 6
        } 
      ],
      "Tyrannosaurus Hives": [
        {
          "filename": "The_Hives_-_Two-Timing_Touch_And_Broken_Bo.mp3",
          "filesize": 2908160,
          "title": "Two-Timing Touch And Broken Bones",
          "track": 2
        },
        {
          "filename": "The_Hives_-_Diabolic_Scheme.mp3",
          "filesize": 4341839,
          "title": "Diabolic Scheme",
          "track": 8
        }
      ]
    }
  }
}
```

## `POST` to `/upload`
**Parameters**: **`file_data`** The raw file data.

Uploads a file to the file store. ID3 tags are parsed on the server and the metadata database is updated accordingly.<br />
(See `example-post.py` for an example.)

## `GET` to `/get_file/`_filename_
**Parameters**: **`filename`** The name of the file (as provided by `/ls` above).

Serves a raw file given by the filename through HTTP.
