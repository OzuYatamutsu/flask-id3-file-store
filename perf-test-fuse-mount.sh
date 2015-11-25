#!/bin/bash
NUM_SAMPLES=1000
TEST_FILE=test_file.mp3

# Prepare testing environment
TEST_FOLDER=testenv_$(date +%s.%N)
TEST_SOURCE=$TEST_FOLDER/file_source
TEST_DESTINATION=$TEST_FOLDER/file_destination
mkdir $TEST_FOLDER
mkdir $TEST_SOURCE
mkdir $TEST_DESTINATION

echo "The following runtime deltas are reported in seconds."
echo "Collecting $NUM_SAMPLES samples..."

# Unmount + build project
prepare_build() {
    fusermount -u $TEST_DESTINATION > /dev/null
    make clean
    make all
}

# Generate NUM_SAMPLES distinct metadata files from test file
prepare_environment() {
    python3 dep_perf-test-id3-replicate.py
    for (( i=1; i<=$NUM_SAMPLES; i++ )) do
        mv test_file$i.mp3 $TEST_SOURCE
    done
}

# Cleanup generated files
test_file_cleanup() {
    for (( i=1; i<=$NUM_SAMPLES; i++ )) do
        rm -fv test_file$i.mp3
    done
}

# Performance run
perf_test() {
    # Time before
    X0=$(date +%s.%N)

    # Mount filesystem
    ./yourtuneslib $TEST_DESTINATION

    # Time after
    X1=$(date +%s.%N)

    # Time delta
    delta=$(python -c "print($X1 - $X0)")
    echo $delta,
}

prepare_build
prepare_environment

# Do performance test NUM_SAMPLES times
for (( i=1; i<=$NUM_SAMPLES; i++ )) do
    perf_test
    mv -f $TEST_SOURCE/test_file$i.mp3 $TEST_DESTINATION
    sleep 0.2
    fusermount -u $TEST_DESTINATION
done

# Clean up when done
./yourtuneslib $TEST_DESTINATION

for (( i=1; i<=$NUM_SAMPLES; i++ )) do
    rm -f "$TEST_DESTINATION/albums/Project 3/$(($i+1)) - Test Song$i.mp3"
done

sleep 1
fusermount -u $TEST_DESTINATION
make clean > /dev/null
rm -Rf $TEST_FOLDER
