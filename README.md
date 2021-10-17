# Rift

Yet another reproduction of muduo library.

### Modifications

Replace `boost` and `muduo` library with C++11/17 features and `glog` library.

* Use `std::chrono` as time utility.
* Replace `scoped_ptr` with `unique_ptr`.
* Modify data structures in *TimerQueue*.
* Move *Socket* to *NewConnectionCallback* instead of pass raw connection fd.

### Logging Convention

* LOG(FATAL) - Terminates the program.
* LOG(ERROR) - Always an error.
* LOG(WARNING) - Something unexpected, but probably not an error.
* Verbose Info Logging has multiple levels:
    * VLOG(0) - Generally useful for this to always be visible to an operator.
    * VLOG(4) - Debug level verbosity.
    * VLOG(5) - Trace level verbosity.
