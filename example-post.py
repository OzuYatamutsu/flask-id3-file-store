from sys import argv
from requests import post
from pprint import pprint

if len(argv) == 2:
    pprint(
        post("http://localhost:9880/upload", files={"file_data": open(argv[1], "rb")})
    )
else:
    print("Usage: python3 example-post.py <path_to_mp3_file>")