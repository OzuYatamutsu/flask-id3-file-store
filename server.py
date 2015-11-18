from flask import Flask, request
from werkzeug import secure_filename
from os import path
from json import load # Load config file

import MySQLdb # Case-sensitive. Worst name ever 
import eyed3 # MP3-only ID3 tag parsing

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
        return "Got data! Filename=" + filename # Debug
    return "Upload failure!"

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
    
    id3_file = eyed3.load(path.join(app.config["UPLOAD_FOLDER"], filename))
    track = id3_file.tag.track_num
    title = id3_file.tag.title
    artist = id3_file.tag.artist
    album = id3_file.tag.album
    year = id3_file.tag.year
    genre = id3_file.tag.genre
    track_comment = id3_file.tag.comment
    
    cursor.execute(
        """INSERT INTO ytfs_meta (
            filename, track, title, artist, 
            album, year, genre, track_comment
        ) VALUES (
            '%s', %s, '%s', '%s',
            '%s', '%s', '%s', '%s'
        )""", (
            filename, track, title, artist,
            album, year, genre, track_comment
        )
    )

def load_config():
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
    print("Starting server on port " + str(SERV_PORT) + "...")
    app.run(
        host = "0.0.0.0",
        port = SERV_PORT
    )
