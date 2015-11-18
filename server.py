from flask import Flask, request, jsonify
from werkzeug import secure_filename
from os import path
from json import load # Load config file
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
        filename = secure_filename(file.filename)
        file.save(path.join(app.config["UPLOAD_FOLDER"], filename))
        db_insert_file(filename, file)
        return "Got data! Filename=" + filename # Debug
    return "Upload failure!"

@app.route("/ls")
def ls():
    """Lists all files in the datastore recursively by album and decade."""
    ls_dict = {"albums": {}, "decades": {}}
    query = "SELECT filename, track, title, artist, album, year FROM ytfs_meta;"
    cursor.execute(query)
    
    for (filename, track, title, artist, album, year) in cursor:
        if album not in ls_dict["albums"]:
            ls_dict["albums"][album] = []
        ls_dict["albums"][album].append({"track": track, "title": title, "path": request.url_root + path.join(app.config["UPLOAD_FOLDER"], filename)})

        decade = 0
        try:
            decade = int(year)
            decade = str(decade - (decade % 10))
        except ValueError:
            # Couldn't parse the year
            decade = "Misc."
        
        if decade not in ls_dict["decades"]:
            ls_dict["decades"][decade] = {}
        if album not in ls_dict["decades"][decade]:
            ls_dict["decades"][decade][album] = []
        ls_dict["decades"][decade][album].append({"track": track, "title": title, "path": request.url_root + path.join(app.config["UPLOAD_FOLDER"], filename)})

    return jsonify(**ls_dict)

@app.route("/get_file", methods=["GET"])
def get_file():
    pass

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
 
    filename = filename.replace("'", "\\'") 
    id3_file = None
    
    try: 
        id3_file = read_tag(path.join(app.config["UPLOAD_FOLDER"], filename))
    except NoTagError:
        # No ID3 tags whatsoever
        print("Inserting misc file: " + filename)
        query = "INSERT INTO ytfs_meta (filename) VALUES '{0}');".format(filename)
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
    query = "INSERT INTO ytfs_meta (filename, track, title, artist, album, year, genre, track_comment) VALUES ('{0}', {1}, '{2}', '{3}', '{4}', '{5}', '{6}', '{7}');".format(filename, track, title, artist, album, year, genre, track_comment)
    print(query) # Debug
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
