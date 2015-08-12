// #include <boost/interprocess/shared_memory_object.hpp>
#include <vector>
namespace stat_log
{
   //TODO: templates?
   class shared_mem_backend
   {
      public:
         void setParams(const std::string& name, size_t size)
         {
            memory.resize(size);
         }

         char* getMemoryPtr()
         {
            return memory.data();
         }

      private:
         // std::shared_ptr<boost::interprocess::shared_memory_object> shm_ptr;
         std::vector<char> memory;
   };
}


#if 0
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace boost::interprocess;
using std::chrono::seconds;

int main(int argc, char** argv)
{
   using namespace std;
   cout << argc << std::endl;
   std::string name {argv[1]};
   shared_memory_object shm_obj
      (open_or_create
       ,name.c_str()
       ,read_write                   //read-write mode
      );

   shm_obj.truncate(1000);

   mapped_region region{shm_obj, read_write};

   char* ptr = (char*)region.get_address();

   while(1)
   {
      cout << "Pattern\n";
      for(int i = 0; i < 100; ++i)
         ptr[i] = i;
      std::this_thread::sleep_for(seconds(1));
      cout << "Zeros\n";
      std::memset(ptr, 0, 100);
      std::this_thread::sleep_for(seconds(1));
   }
   return 0;
}
#endif

