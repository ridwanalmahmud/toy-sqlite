### Prerequisite
```
pacman -S make gcc
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
- Build the tests
```
make build-test
```
- Run the tests
```
make run-test
```
- Build and run the tests
```
make test
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
