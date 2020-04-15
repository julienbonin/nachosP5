#include "AccessMatrix.h"
 #include "AccessList.h"
 #include "CapabilityList.h"
 #include "Security.h"

 enum {Access_Matrix=1, Access_List, Capability_List};

 //int numDomains = (Random() % 5) + 3; // # of Domains [Threads]
 //int numObjects = (Random() % 5) + 3; // # of Object [Character Buffers]

 void Security(int method) {
     switch (method)
     {
     case Access_Matrix: {
         AccessMatrix();
         break;
     }
     case Access_List: {
         AccessList();
         break;
     }
     case Capability_List: {
         CapabilityList();
         break;
     }
     }
 } 
