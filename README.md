# Rift

Yet another reproduction of muduo library.

### Modifications

Replace `boost` and `muduo` library with C++11 features and `glog` library.

* Use `std::chrono` as time utility.
* Use `unique_ptr` manage *Timer*s in *TimerQueue*.
* Replace `set` with `multimap` in *TimerQueue*.