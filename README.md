# Rift

Yet another reproduction of muduo library.

### Modifications

Replace `boost` and `muduo` library with C++11 features and `glog` library.

* Use `std::chrono` as time utility.
* Replace `scoped_ptr` with `unique_ptr`.
* Replace `set` with `multimap` in *TimerQueue*.
* Move *Socket* to *NewConnectionCallback* instead of pass raw connection fd.
 
