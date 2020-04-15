#include "AccessList.h"
 #include "system.h"

 struct Obj {
     char *Object;
     int *Domains;
     int *Permissions;
 };

 int numDomainsAL = (Random() % 5) + 3; // # of Domains [Threads]
 int numObjectsAL = (Random() % 5) + 3; // # of Object [Character Buffers]
 Obj **ObjectList = new Obj *[numObjectsAL+numDomainsAL];
 enum {NONE, READ, WRITE, ReadWrite, Switch};
 char* printEntryAL(int n);
 void StartProcessing_AL(int n);
 Semaphore **ObjectSemaphoresAL = new Semaphore*[numObjectsAL];
 bool arbitratorAL(int X, int action);

 // Main function for AccessList
 void AccessList() {

     int r;
     for (int i=0; i<numDomainsAL; i++) {
     printf("            Domain%d", i);
     }
     printf("\n");

     for (int i=0; i<numObjectsAL+numDomainsAL; i++) {
         if (i < numObjectsAL) {
             ObjectList[i] = new Obj();
             ObjectList[i]->Object = new char[6];
              ObjectList[i]->Domains = new int[numDomainsAL];
             ObjectList[i]->Permissions = new int[numDomainsAL];
             printf("Object%d     ", i);
             for (int j=0; j<numDomainsAL;j++) {
                 ObjectList[i]->Domains[j] = j;
                 r = (Random() % 4);
                 ObjectList[i]->Permissions[j] = r==1 ? READ : r==2 ? WRITE : r==3 ? ReadWrite : NONE;
                 printf("%-19s", printEntryAL(r));
             }
             printf("\n");
             char *buffer = new char[6];
             for (int j=0; j<5;j++) {
                 buffer[j] = char((Random() % 26) + 65); // Generates a random character to add to buffer
             }
             buffer[5] = '\0';
             ObjectList[i]->Object = buffer;
             Semaphore *s = new Semaphore(buffer, 1);
             ObjectSemaphoresAL[i] = s;
         } else { // Here, object list contain Domain switching info
             ObjectList[i] = new Obj();
             ObjectList[i]->Domains = new int[numDomainsAL];
             ObjectList[i]->Permissions = new int[numDomainsAL];
             printf("Domain%d     ", i-numObjectsAL);
             for (int j=0; j<numDomainsAL;j++) {
                 ObjectList[i]->Domains[j] = j;
                 r = i!=j+numObjectsAL ? (Random() % 2) : 0;
                 ObjectList[i]->Permissions[j] = r==1 ? Switch : NONE;
                 printf("%-19s", printEntryAL(ObjectList[i]->Permissions[j]));
             }
             printf("\n");
         }
     }

     printf("\n  Object List\n");
     for (int i=0; i<numObjectsAL; i++) {
         printf("Object%d  %s\n", i, ObjectList[i]->Object);
     }

     Thread *Domains[numDomainsAL];
     for (int i=0; i<numDomainsAL; i++) {
         //char *s = new char(i+48);
         char *s = new char[6];
         s[0] = 'U';
         s[1] = 's';
         s[2] = 'e';
         s[3] = 'r';
         s[4] = char(48+i);
         s[5] = '\0';
         Domains[i] = new Thread(s);
         Domains[i]->setDomain(i);
         printf("%s has been created and is in Domain %d\n", Domains[i]->getName(), Domains[i]->getDomain());
     }

     IntStatus old = interrupt->SetLevel(IntOff);
     for (int i=0; i<numDomainsAL;i++) {
         Domains[i]->Fork(StartProcessing_AL, 0);
         printf("%s has been forked.\n", Domains[i]->getName());
     }
     (void) interrupt->SetLevel(old);


 }


 void StartProcessing_AL(int n) {
     int action;
     int X;
     for (int i=0; i<5;i++) { // Each thread should try to access objects at least 5 times
         X = Random() % (numDomainsAL+numObjectsAL); // colomn (object) to act upon
         if (X < numObjectsAL) {
             action = (Random() % 2) + 1; // Should thread Read or Write on object X
             //printf("Action: %d\n", action);
             if (arbitratorAL(X, action)) {
                 ObjectSemaphoresAL[X]->P();
                 printf("%s has been granted access to object %d\n", currentThread->getName(), X);
                 if (action == READ) {
                     printf("%s reads %s from Object%d\n", currentThread->getName(), ObjectList[X]->Object, X);
                 } else if (action == WRITE) {
                     printf("Object%d old value: %s changed to ", X, ObjectList[X]->Object);
                     int randomIndex = Random() % 5; // character index in string to change
                     //char randomChar = (char)((Random() % 26 ) + 65); // character to change to in string
                     //printf("Random char: %c", randomChar);
                     ObjectList[X]->Object[randomIndex] = (char)((Random() % 26 ) + 65); // character to change to in string
                     printf("%s by %s\n", ObjectList[X]->Object, currentThread->getName());
                 }
                 int RandomYeild = (Random() % 5)+3;
                 printf("%s will yeild %d times\n", currentThread->getName(), RandomYeild);
                 for (int s=0;s<RandomYeild;s++) {
                     currentThread->Yield();
                 }
                 ObjectSemaphoresAL[X]->V();
             } else {
                 printf("%s was denied access to Object%d\n", currentThread->getName(), X);
             }
         } else if (X >= numObjectsAL && (X-numObjectsAL) != currentThread->getDomain()) {
             if (arbitratorAL(X, 4)) {
                 X = X - numObjectsAL;
                 printf("%s has been granted access to switch from Domain %d to Domain %d\n", currentThread->getName() ,currentThread->getDomain(), X);
                 currentThread->setDomain(X);
                 printf("%s has switched to domain %d\n", currentThread->getName(), X);
                 int RandomYeild = (Random() % 5)+3;
                 printf("%s will yeild %d times\n", currentThread->getName(), RandomYeild);
                 for (int s=0;s<RandomYeild;s++) {
                     currentThread->Yield();
                 }
             } else {
                 printf("%s was denied access to switch to domain %d\n", currentThread->getName(), X-numObjectsAL);
             }
         }
         currentThread->Yield();
     }
 }

 bool arbitratorAL(int X, int action) {
     if (action == 1 || action == 2) {
         printf("%s in domain %d is attempting to %s on object %d\n", currentThread->getName(), currentThread->getDomain(), action==1 ? "Read" : "Write",X);
     } else {
         printf("%s in domain %d is attempting to %s to domain %d\n", currentThread->getName(), currentThread->getDomain(), "Switch" , X-numObjectsAL);
     }
     int permission = ObjectList[X]->Permissions[currentThread->getDomain()];
     return  permission == action ? 1 : permission == ReadWrite ? 1 : 0;
 }

 char* printEntryAL(int n) {
     enum {NONE, Read, Write, ReadWrite, Switch};
     switch (n) {
         case NONE:
             return " ";
         case Read:
             return "Read";
         case Write:
             return "Write";
         case ReadWrite:
             return "ReadWrite";
         case Switch:
             return "Switch";
         }
     return "Error";
 }
