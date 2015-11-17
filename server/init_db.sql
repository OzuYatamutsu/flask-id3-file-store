CREATE TABLE ytfs_meta (
    filename VARCHAR(256) UNIQUE NOT NULL,
    track TINYINT UNSIGNED,
    title VARCHAR(256),
    artist VARCHAR(256),
    album VARCHAR(256),
    year VARCHAR(256),
    genre VARCHAR(256),
    track_comment VARCHAR(256),

    PRIMARY KEY (filename)
);
