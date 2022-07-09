# How a std::weak_ptr can prevent full memory cleanup

Test the impact on using std::make_shared and a raw new in combination with std::shared_ptr and std::weak_ptr.

Two chunks of data are tested that are managed differently. The std::shared_ptr mechanics are the same, but the impact on memory cleanup are different.

For more information see [this article](https://www.codeproject.com/Articles/1227690/How-a-weak-ptr-Might-Prevent-Full-Memory-Cleanup-o).
