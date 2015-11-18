# project3
YourTunes File System

This project requires `libfuse-dev` and `pkg-config` to be installed, or the Makefile will complain.
We also need `mp3info` to read MP3 ID3 tags.

//TESTING INFO
change cache_manager.c getCachePath() to return path to a mp3 on your system

change yourtuneslib.c ytl_getattr stbuf->st_size to the size of the file

make and run yourtuneslib 

Song_Name.mp3 should be listed in mounted directory, opening it should play it.
