### Prerequisite
```
pacman -S make clang gtest
```

### Makefile

- Make the executable
```
make
```
- Make and Run the executable
```
make run <db_name>
```
- Build and run the tests
```
make test
```
- Remove the object files directory
```
make clean-obj
```
- Remove the test executable
```
make clean-test
```
- Remove the executables and the obj dir
```
make clean
```

### Supported commands
- Print the constants
```
.constants
```
- View the internal btree structure
```
.btree
```
- Insert into the database
```
insert <id> <key> <value>
```
- To print out the database
```
.select
```
- To save and exit the database
```
.exit
```
