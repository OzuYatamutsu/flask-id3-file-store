#!/bin/bash
NUM_SAMPLES=50
TEST_FILE=test_file.mp3

# Prepare testing environment
TEST_FOLDER=testenv_$(date +%s.%N)
TEST_SOURCE=$TEST_FOLDER/file_source
TEST_DESTINATION=$TEST_FOLDER/file_destination
mkdir $TEST_FOLDER
mkdir $TEST_SOURCE
mkdir $TEST_DESTINATION

# Move file into source
cp -fv $TEST_FILE $TEST_SOURCE

echo "The following runtime deltas are reported in seconds."
echo "Collecting $NUM_SAMPLES samples..."

# Cleanup destination each time
prepare_test() {
    rm -Rf $TEST_DESTINATION/*
}

# Performance run
perf_test() {
    # Time before
    X0=$(date +%s.%N)

    # Copy file from source to destination
    cp -fv $TEST_SOURCE/$TEST_FILE $TEST_DESTINATION

    # Time after
    X1=$(date +%s.%N)

    # Time delta
    delta=$(python -c "print($X1 - $X0)")
    echo $delta,
}

# Do performance test NUM_SAMPLES times
for (( i=1; i<=$NUM_SAMPLES; i++ )) do
    prepare_test > /dev/null
    perf_test
done

# Clean up when done
make clean > /dev/null
rm -Rfv $TEST_FOLDER