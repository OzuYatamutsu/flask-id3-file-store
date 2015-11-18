#!/bin/bash
DEF_ROOT_PASS=team14

# Root credentials for MySQL init
sudo debconf-set-selections <<< 'mysql-server mysql-server/root_password password $DEF_ROOT_PASS'
sudo debconf-set-selections <<< 'mysql-server mysql-server/root_password_again password $DEF_ROOT_PASS'

# Download required packages
sudo apt-get install -y python3 python3-pip mysql-server libmysqlclient-dev

# Get required pip packages
sudo pip3 install flask mysqlclient eyeD3

# Set up database
mysql -u root --password=$DEF_ROOT_PASS -e "CREATE DATABASE IF NOT EXISTS ytfs"
mysql -u root --password=$DEF_ROOT_PASS ytfs < init_db.sql

# Set up filesystem
mkdir ./data
