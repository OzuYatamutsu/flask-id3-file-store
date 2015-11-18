#!/bin/bash

# Username and password for non-root database user.
YTFS_USERNAME=ytfs_agent
YTFS_PASS=team14

# Used if MySQL is not installed.
DEF_ROOT_PASS=team14

# Check if MySQL is installed
dpkg -s mysql-server > /dev/null

# MySQL is not installed
if [ $? -neq 0 ]; then 
    echo "MySQL server is not installed. Setting up with default password."

    # Root credentials for MySQL init
    sudo debconf-set-selections <<< 'mysql-server mysql-server/root_password password $DEF_ROOT_PASS'
    sudo debconf-set-selections <<< 'mysql-server mysql-server/root_password_again password $DEF_ROOT_PASS'
    sudo apt-get install mysql-server -y

    # Set up database
    echo "Setting up YTFS metadata tables."
    mysql -u root --password=$DEF_ROOT_PASS -e "CREATE DATABASE IF NOT EXISTS ytfs"
    mysql -u root --password=$DEF_ROOT_PASS ytfs < init_db.sql

    # Add non-root account
    echo "Adding YTFS user."
    mysql -u root --password=$DEF_ROOT_PASS -e "CREATE USER '$YTFS_USERNAME'@'%' IDENTIFIED BY '$YTFS_PASS';"
    mysql -u root --password=$DEF_ROOT_PASS -e "GRANT SELECT,INSERT,UPDATE,DELETE ON ytfs.ytfs_meta TO '$YTFS_USERNAME'@'%' IDENTIFIED BY '$YTFS_PASS';"
else 
    echo "MySQL server is installed. Prepare to enter your MySQL root password!"
    
    # Set up database
    echo "Setting up YTFS metadata tables."
    mysql -u root -p -e "CREATE DATABASE IF NOT EXISTS ytfs"
    mysql -u root -p ytfs < init_db.sql

    # Add non-root account
    echo "Adding YTFS user."
    mysql -u root -p -e "CREATE USER '$YTFS_USERNAME'@'%' IDENTIFIED BY '$YTFS_PASS';"
    mysql -u root -p -e "GRANT SELECT,INSERT,UPDATE,DELETE ON ytfs.ytfs_meta TO '$YTFS_USERNAME'@'%' IDENTIFIED BY '$YTFS_PASS';"
fi

# Download remaining required packages
sudo apt-get install -y python3 python3-pip libmysqlclient-dev

# Get required pip packages
sudo pip3 install flask mysqlclient eyeD3

# Set up filesystem
mkdir ./data
