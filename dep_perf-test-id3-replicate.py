from stagger import read_tag
from stagger.id3 import *
from shutil import copyfile

TEST_FILE = "test_file.mp3"
NUM_SAMPLES = 10000

def main():
    # Make NUM_SAMPLES copies of test file
    duplicate_files() 
    # Make NUM_SAMPLES distinct metadata for each file
    overwrite_meta()

def duplicate_files():
    for time in range(1, NUM_SAMPLES + 1):
        copyfile(TEST_FILE, TEST_FILE.replace(".", str(time) + "."))

def overwrite_meta():
    for time in range(1, NUM_SAMPLES + 1):
        id3_file = read_tag(TEST_FILE.replace(".", str(time) + "."))
        id3_file.title = id3_file.title + str(time)
        id3_file.track = id3_file.track + time
        id3_file.write()
main()
