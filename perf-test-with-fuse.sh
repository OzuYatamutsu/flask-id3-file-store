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
cp -f $TEST_FILE $TEST_SOURCE

echo "The following runtime deltas are reported in seconds."
echo "Collecting $NUM_SAMPLES samples..."

# Unmount + build project from scratch each time
prepare_build() {
    fusermount -u $TEST_DESTINATION > /dev/null
    make clean
    make all
}

# Cleanup destination each time and mount folder
prepare_test() {
    mysql -u root --password=team14 ytfs "DELETE FROM ytfs_meta;"
    rm -f server/data/*
    ./yourtuneslib $TEST_DESTINATION
}

# Performance run
perf_test() {
    # Time before
    X0=$(date +%s.%N)

    # Copy file from source to destination
    cp -f $TEST_SOURCE/$TEST_FILE $TEST_DESTINATION

    # Time after
    X1=$(date +%s.%N)

    # Time delta
    delta=$(python -c "print($X1 - $X0)")
    echo $delta,
}

# Do performance test NUM_SAMPLES times
for (( i=1; i<=$NUM_SAMPLES; i++ )) do
    prepare_build > /dev/null
    prepare_test > /dev/null
    perf_test
    sleep 0.5
done

# Clean up when done
prepare_test > /dev/null
fusermount -u $TEST_DESTINATION > /dev/null
make clean > /dev/null
rm -Rf $TEST_FOLDER
