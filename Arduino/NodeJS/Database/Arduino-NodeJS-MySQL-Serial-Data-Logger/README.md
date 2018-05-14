# Send values over Serial and insert into a database

This is an example that shows how to insert data sent over the Serial port (USB) of an Arduino into a MySQL database.

It uses a small Node JS application to connect to the Arduino and to the database, and receive and parse the data received from the Arduino before inserting it into the database.

## Install necessary software

### Node JS

The easiest way is to install Node JS and the npm package manager from the Ubuntu repositories.
```sh
sudo apt-get update
sudo apt-get install nodejs npm
```
If you're using a different operating system, you can download the installer from the [Node JS website](https://nodejs.org/en/).  
Take a look at [this page](https://www.digitalocean.com/community/tutorials/how-to-install-node-js-on-ubuntu-16-04) for alternative installation options for Ubuntu.

### Node JS packages

You need extra packages for using serial ports, and for connecting to a MySQL server.
```sh
npm install serialport mysql
```

### MySQL

If you haven't already, install a database server, like MySQL or MariaDB.  
[How To Install MySQL on Ubuntu 16.04](https://www.digitalocean.com/community/tutorials/how-to-install-mysql-on-ubuntu-16-04)

If you're on Windows or Mac, you can use software like XAMPP or similar.

## Create a database

Run the `MySQL/Arduino.sql` file on your database server. It will create a database `Arduino` with a table `Data` that has three columns: `id`, which is a unique index for the entry, `data` containing up to 255 characters of text, and `time`, a time stamp of when the record was created.

```sql
CREATE DATABASE IF NOT EXISTS `Arduino`
  DEFAULT CHARACTER SET = utf8mb4
  DEFAULT COLLATE = utf8mb4_general_ci;

USE `Arduino`;

CREATE TABLE IF NOT EXISTS `Data` (
  `id` int(11) AUTO_INCREMENT NOT NULL,
  PRIMARY KEY (`id`),
  `data` varchar(255) NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE = InnoDB 
  DEFAULT CHARSET = utf8mb4
  DEFAULT COLLATE = utf8mb4_general_ci;
```
Also set a root password for security, and create a new user for the Arduino, also secured with a password, of course.

## Upload the Arduino code

An example Arduino sketch is included, that just prints "Hello, World!" and then the value of A0 every ten seconds.  
Open it in the Arduino IDE, and upload it to the board.

## Run the Node JS application

First change the database user and password information in `NodeJS/main.js` and save it.

To run the application, just open a terminal, and run
```sh
./main.js
```
If you get an error, try making it executable first:
```sh
chmod +x main.js
```
Alternatively, launch Node JS explicitly:
```sh
node main.js
```

## Inspect the inserted data

Select the data in the database using SQL.

```sql
SELECT * FROM `Arduino`.`Data`;
```
```
+----+--------+---------------------+
| id | data   | time                |
+----+--------+---------------------+
|  1 | Hello, | 2018-05-14 12:00:00 |
|  2 | World! | 2018-05-14 12:00:00 |
|  3 | 595    | 2018-05-14 12:00:10 |
|  4 | 399    | 2018-05-14 12:00:20 |
|  5 | 395    | 2018-05-14 12:00:30 |
|  6 | 394    | 2018-05-14 12:00:40 |
|  7 | 394    | 2018-05-14 12:00:50 |
|  8 | 400    | 2018-05-14 12:01:00 |
|  9 | 408    | 2018-05-14 12:01:10 |
| 10 | 398    | 2018-05-14 12:01:20 |
| 11 | 395    | 2018-05-14 12:01:30 |
| 12 | 396    | 2018-05-14 12:01:40 |
| 13 | 407    | 2018-05-14 12:01:50 |
| 14 | 408    | 2018-05-14 12:02:00 |
| 15 | 395    | 2018-05-14 12:02:10 |
| 16 | 394    | 2018-05-14 12:02:20 |
+----+--------+---------------------+
```