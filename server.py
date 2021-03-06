from flask import Flask, request, jsonify, send_from_directory
from os import path, stat, remove
from json import load # Load config file
from hashlib import sha1 # For hashing file data
from stagger import read_tag # MP3 tag parsing
from stagger.id3 import *
from stagger.errors import NoTagError # Catch if no ID3 tags to read
import MySQLdb # Case-sensitive. Worst name ever 

# Config vars - will be populated from config file
CONFIG_FILE = "./config.json"
SERV_PORT = 0
DATA_DIR = ""
DB_HOST = ""
DB_PORT = 0
DB_USER = ""
DB_PASSWD = "" 
DB_DB = ""

app = Flask(__name__)
db = None
cursor = None

@app.route("/")
def ytfs_hello():
    return "Hiya! This is the Flask web server powering the YTFS data store."

@app.route("/upload", methods=["POST"])
def upload():
    """Uploads a file to the YTFS data store.
    POST to /upload with a file_data field."""
    file = request.files["file_data"]
    if file:
        filename = hash_file(file) + file.filename[-4:] # e.g. file.mp3 -> (hash(file)).mp3
        file.save(path.join(app.config["UPLOAD_FOLDER"], filename))
        db_insert_file(filename, file)
        return "0"
    return "-1"

@app.route("/ls")
def ls():
    """Lists all files in the datastore recursively by album and decade."""
    ls_dict = {"albums": {}, "decades": {}}
    query = "SELECT filename, track, title, artist, album, year FROM ytfs_meta ORDER BY track ASC;"
    cursor.execute(query)
    
    for (filename, track, title, artist, album, year) in cursor:
        album = album if album != "" else "Unknown"
        if album not in ls_dict["albums"]:
            ls_dict["albums"][album] = []
        ls_dict["albums"][album].append({
            "track": track, 
            "title": title, 
            "filename": filename,
            "filesize": stat(path.join(app.config["UPLOAD_FOLDER"], filename)).st_size
        })

        decade = 0
        try:
            decade = int(year)
            decade = str(decade - (decade % 10)) + 's'
        except (ValueError, TypeError):
            # Couldn't parse the year (e.g. no metadata or strange date format)
            decade = "Unknown"
        
        if decade not in ls_dict["decades"]:
            ls_dict["decades"][decade] = {}
        if album not in ls_dict["decades"][decade]:
            ls_dict["decades"][decade][album] = []
        ls_dict["decades"][decade][album].append({
                "track": track, 
                "title": title, 
                "filename": filename,
                "filesize": stat(path.join(app.config["UPLOAD_FOLDER"], filename)).st_size
        })

    return jsonify(**ls_dict)

@app.route("/get_file/<path:filename>")
def get_file(filename):
    """Retrieves a file."""

    return send_from_directory(app.config["UPLOAD_FOLDER"], filename)

@app.route("/delete_file/<path:filename>")
def delete_file(filename):
    """Deletes a file."""
   
    try:
        remove(path.join(app.config["UPLOAD_FOLDER"], filename)) 
        db_delete_file(filename)
    except FileNotFoundError:
        return "{0} does not exist on server.".format(filename)
    except:
        return "Could not delete {0}.".format(filename)
    return "Deleted {0}.".format(filename)
    
@app.route("/web")
def web():
    """Provides the web interface."""
    json_ls = "<script>\nvar json_ls = " + ls().get_data().decode("utf-8") + "\n</script>\n\n"
    f = open('web.html', 'r')
    html_content = f.read()
    f.close()
    return json_ls + html_content

def hash_file(file):
    """Computes a SHA1 hash of a given file."""
    file_data = file.read()
    file.seek(0) # Resets read pointer
    return sha1(file_data).hexdigest()

def db_connect():
    """Attempts to connect to the configured database.
    Returns a db and a cursor object."""    
    db = MySQLdb.connect(
        host = DB_HOST,
        port = DB_PORT,
        user = DB_USER,
        passwd = DB_PASSWD,
        db = DB_DB
    )

    return db, db.cursor()

def db_insert_file(filename, file):
    """Reads file metadata and inserts it into the database."""
 
    id3_file = None
    
    try: 
        id3_file = read_tag(path.join(app.config["UPLOAD_FOLDER"], filename))
    except NoTagError:
        # No ID3 tags whatsoever
        print("Inserting misc file: " + filename)
        query = "INSERT IGNORE INTO ytfs_meta (filename) VALUES ('{0}');".format(filename)
        cursor.execute(query)
        db.commit()
        return

    track = id3_file.track if id3_file.track is not None else 0
    title = id3_file.title.replace("'", "\\'")
    artist = id3_file.artist.replace("'", "\\'")
    album = id3_file.album.replace("'", "\\'")
    year = id3_file.date.replace("'", "\\'")
    genre = id3_file.genre.replace("'", "\\'")
    track_comment = id3_file.comment.replace("'", "\\'")
   
    print("Inserting: " + artist + " - " + title) 
    query = "INSERT IGNORE INTO ytfs_meta (filename, track, title, artist, album, year, genre, track_comment) " + \
        "VALUES ('{0}', {1}, '{2}', '{3}', '{4}', '{5}', '{6}', '{7}');".format( \
        filename, track, title, artist, album, year, genre, track_comment
    )
    cursor.execute(query)
    db.commit() # Save changes back to DB
    
def db_delete_file(filename):
    """Deletes file metadata from the database."""

    query = "DELETE FROM ytfs_meta WHERE filename = '{0}'".format(filename)
    cursor.execute(query)
    db.commit() # Save changes back to DB

def load_config(
):
    global SERV_PORT, DATA_DIR, DB_HOST, DB_PORT, DB_USER, DB_PASSWD, DB_DB
    config_data = None

    with open(CONFIG_FILE) as config:
        config_data = load(config)
    
    SERV_PORT = int(config_data["server"]["port"])
    DATA_DIR = config_data["server"]["data_dir"]
    DB_HOST = config_data["db"]["host"]
    DB_PORT = config_data["db"]["port"]
    DB_USER = config_data["db"]["username"]
    DB_PASSWD = config_data["db"]["password"]
    DB_DB = config_data["db"]["db"]

if __name__ == "__main__":
    load_config()
    app.config["UPLOAD_FOLDER"] = DATA_DIR
    db, cursor = db_connect()
    if db: print("Connected to database!")
    app.run(
        host = "0.0.0.0",
        port = SERV_PORT
    )
