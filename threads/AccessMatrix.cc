#include "AccessMatrix.h"

 int numDomainsAM = (Random() % 5) + 3; // # of Domains [Threads]
 int numObjectsAM = (Random() % 5) + 3; // # of Object [Character Buffers]
 int **AccessMat = new int *[numDomainsAM];
 char **ObjectsAM = new char *[numObjectsAM];
 enum {NONE, READ, WRITE, ReadWrite, Switch};
 Semaphore **ObjectSemaphores = new Semaphore*[numObjectsAM];

 void AccessMatrix() {

     // Print numbejr of Domains and Objects
     printf("Number of Domains: %d\n", numDomainsAM);
     printf("Number of Objects: %d\n", numObjectsAM);

     // Complete creation of AccessMatrix
     for (int i=0; i < numDomainsAM;i++) {
         AccessMat[i] = new int[numObjectsAM+numDomainsAM];
     }

     // Fill in Access Matrix entries randomly
     int r;
     int val;
     for (int i=0; i<numDomainsAM;i++) { // i represents domain [row]
         for (int j=0; j<numObjectsAM+numDomainsAM;j++) { // j represents object [columb]
             if (j<numObjectsAM) {
                 r = (Random() % 4);
                 val = r==1 ? READ : r==2 ? WRITE : r==3 ? ReadWrite : NONE;
                 AccessMat[i][j] = val; // decide NONE, Read, Write, ReadWrite
             } else if (j>=numObjectsAM) {
                 r = i!=j ? (Random() % 2) : 0;
                 AccessMat[i][j] = r==1 ? Switch : NONE;
             }
         }
     }
     printMatrix(AccessMat); // Prints Access Matrix


     // Create List for Objects [randomly]
     printf("Object Strings\n");
     for (int i=0; i<numObjectsAM; i++) {
         char *buffer = new char[6];
         for (int j=0; j<5;j++) {
             buffer[j] = char((Random() % 26) + 65); // Generates a random character to add to buffer
         }
         buffer[5] = '\0';
         ObjectsAM[i] = buffer;
         Semaphore *s = new Semaphore(buffer, 1);
         ObjectSemaphores[i] = s;
         printf("Object%d: %s\n", i, ObjectsAM[i]);
     }
     printf("\n");

     // Create threads to represent domains
     Thread *Domains[numDomainsAM];
     for (int i=0; i<numDomainsAM; i++) {
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
     printf("\n");

     // note: I had to disable interupts b/c I was getting unordered output with diff. -rs values.
     IntStatus old = interrupt->SetLevel(IntOff);
     for (int i=0; i<numDomainsAM;i++) {
         Domains[i]->Fork(StartProcessing_AM, 0);
         printf("%s has been forked.\n", Domains[i]->getName());
     }
     (void) interrupt->SetLevel(old);

     printf("\n");
     //for (int i=0; i<5;i++) { currentThread->Yield(); }
 } // End of access matrix

 void StartProcessing_AM(int n) {
     int action;
     int X;
     for (int i=0; i<5;i++) { // Each thread should try to access objects at least 5 times
         X = Random() % (numDomainsAM+numObjectsAM); // colomn (object) to act upon
         if (X < numObjectsAM) {
             action = (Random() % 2) + 1; // Should thread Read or Write on object X
             //printf("Action: %d\n", action);
             if (arbitrator(X, action)) {
                 ObjectSemaphores[X]->P();
                 printf("%s has been granted access to object %d\n", currentThread->getName(), X);
                 if (action == READ) {
                     printf("%s reads %s from Object%d\n", currentThread->getName(), ObjectsAM[X], X);
                 } else if (action == WRITE) {
                     printf("Object%d old value: %s changed to ", X, ObjectsAM[X]);
                     int randomIndex = Random() % 5; // character index in string to change
                     //char randomChar = (char)((Random() % 26 ) + 65); // character to change to in string
                     //printf("Random char: %c", randomChar);
                     ObjectsAM[X][randomIndex] = (char)((Random() % 26 ) + 65); // character to change to in string
                     printf("%s by %s\n", ObjectsAM[X], currentThread->getName());
                 }
                 int RandomYeild = (Random() % 5)+3;
                 printf("%s will yeild %d times\n", currentThread->getName(), RandomYeild);
                 for (int s=0;s<RandomYeild;s++) {
                     currentThread->Yield();
                 }
                 ObjectSemaphores[X]->V();
             } else {
                 printf("%s was denied access to Object%d\n", currentThread->getName(), X);
             }
         } else if (X >= numObjectsAM && (X-numObjectsAM) != currentThread->getDomain()) {
             if (arbitrator(X, 4)) {
                 X = X - numObjectsAM;
                 printf("%s has been granted access to switch from Domain %d to Domain %d\n", currentThread->getName() ,currentThread->getDomain(), X);
                 currentThread->setDomain(X);
                 printf("%s has switched to domain %d\n", currentThread->getName(), X);
                 int RandomYeild = (Random() % 5)+3;
                 printf("%s will yeild %d times\n", currentThread->getName(), RandomYeild);
                 for (int s=0;s<RandomYeild;s++) {
                     currentThread->Yield();
                 }
             } else {
                 printf("%s was denied access to switch to domain %d\n", currentThread->getName(), X-numObjectsAM);
             }
         }
         currentThread->Yield();
     }
 }

 bool arbitrator(int X, int action) {
     if (action == 1 || action == 2) {
         printf("%s in domain %d is attempting to %s on object %d\n", currentThread->getName(), currentThread->getDomain(), action==1 ? "Read" : "Write",X);
     } else {
         printf("%s in domain %d is attempting to %s to domain %d\n", currentThread->getName(), currentThread->getDomain(), "Switch" , X-numObjectsAM);
     }
     int permission = AccessMat[currentThread->getDomain()][X];
     return  permission == action ? 1 : permission == ReadWrite ? 1 :0;
 }

 char* printEntry(int n) {
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

 void printMatrix(int **AccessMatrix) {
             for (int i=0; i<divRoundUp((numObjectsAM+numDomainsAM),2);i++){
                 printf("          ");
             }
             printf("Access Matrix\n\n            ");

             for (int i=0; i<(numObjectsAM);i++) {
                 printf("%-6s%-4d  ", "Object", i);
             }
             for (int i=0; i<(numDomainsAM);i++) {
                 printf("%-6s%-4d  ", "Domain", i);
             }
             printf("\n");

             for (int i=0; i<numDomainsAM;i++) { // i represents domain [row]
                 printf("%-6s%-4d  ", "Domain", i);
                 for (int j=0; j<numDomainsAM+numObjectsAM;j++) { // j represents object [columb]
                     printf("%-10s  ", printEntry(AccessMatrix[i][j]));

                 }
                 printf("\n");
             }
             printf("\n");
         } 
