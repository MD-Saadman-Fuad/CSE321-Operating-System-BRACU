#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_USERS 5 //3 dewa chilo kintu extra r jnno 5
#define MAX_RESOURCES 5
#define MAX_NAME_LEN 20


typedef enum{
    READ = 1,
    WRITE = 2,
    EXECUTE = 4
} Permission;


typedef struct{
    char name[MAX_NAME_LEN];
} User;


typedef struct{
    char name[MAX_NAME_LEN];
} Resource;


typedef struct{
    char userName[MAX_NAME_LEN];
    int permissions;
} ACLEntry;


typedef struct{
    Resource resource;
    ACLEntry aclEntries[MAX_USERS];
    int aclCount;
} ACLControlledResource;


typedef struct{
    char resourceName[MAX_NAME_LEN];
    int permissions;
} Capability;


typedef struct{
    User user;
    Capability capabilities[MAX_RESOURCES];
    int capabilityCount;
} CapabilityUser;


void printPermissions(int perm){
    if (perm == 0) {
        printf("NO PERMISSIONS");
        return;
    }
    int first = 1;
    if (perm & READ) {
        printf("READ");
        first = 0;
    }
    if (perm & WRITE) {
        if (!first) printf(" ");
        printf("WRITE");
        first = 0;
    }
    if (perm & EXECUTE) {
        if (!first) printf(" ");
        printf("EXECUTE");
    }
}


int hasPermission(int userPerm, int requiredPerm){
    return (userPerm & requiredPerm) == requiredPerm;
}


void addACLEntry(ACLControlledResource *res, const char *userName, int permissions) {
    if (res->aclCount < MAX_USERS) {
        strcpy(res->aclEntries[res->aclCount].userName, userName);
        res->aclEntries[res->aclCount].permissions = permissions;
        res->aclCount++;
    }
}


void addCapability(CapabilityUser *user, const char *resourceName, int permissions) {
    if (user->capabilityCount < MAX_RESOURCES) {
        strcpy(user->capabilities[user->capabilityCount].resourceName, resourceName);
        user->capabilities[user->capabilityCount].permissions = permissions;
        user->capabilityCount++;
    }
}


void checkACLAccess(ACLControlledResource *res, const char *userName, int perm){
    int idx = -1;
    for (int i = 0; i < res->aclCount; i++) {
        if (strcmp(res->aclEntries[i].userName, userName) == 0) {
            idx = i;
            break;
        }
    }


    if (idx == -1) {
        printf("ACL Check: User %s has NO entry for resource %s: Access DENIED\n",
               userName, res->resource.name);
        return;
    }


    printf("ACL Check: User %s requests ", userName);
    printPermissions(perm);
    printf(" on %s: ", res->resource.name);


    if (hasPermission(res->aclEntries[idx].permissions, perm)) {
        printf("Access GRANTED\n");
    } else {
        printf("Access DENIED\n");
    }
}


void checkCapabilityAccess(CapabilityUser *user, const char *resourceName, int perm){
    int idx = -1;
    for (int i = 0; i < user->capabilityCount; i++) {
        if (strcmp(user->capabilities[i].resourceName, resourceName) == 0) {
            idx = i;
            break;
        }
    }


    if (idx == -1) {
        printf("Capability Check: User %s has NO capability for %s: Access DENIED\n",
               user->user.name, resourceName);
        return;
    }


    printf("Capability Check: User %s requests ", user->user.name);
    printPermissions(perm);
    printf(" on %s: ", resourceName);


    if (hasPermission(user->capabilities[idx].permissions, perm)) {
        printf("Access GRANTED\n");
    } else {
        printf("Access DENIED\n");
    }
}


int main(){
    User users[MAX_USERS] = {{"Alice"}, {"Bob"}, {"Charlie"}, {"David"}, {"Eva"}};
    Resource resources[MAX_RESOURCES] = {{"File1"}, {"File2"}, {"File3"}, {"File4"}, {"File5"}};


    ACLControlledResource aclResources[MAX_RESOURCES];
    for (int i = 0; i < MAX_RESOURCES; i++) {
        strcpy(aclResources[i].resource.name, resources[i].name);
        aclResources[i].aclCount = 0;
    }


    addACLEntry(&aclResources[0], "Alice", READ | WRITE);
    addACLEntry(&aclResources[0], "Bob", READ);
    addACLEntry(&aclResources[1], "Alice", EXECUTE);
    addACLEntry(&aclResources[1], "Charlie", READ | WRITE);
    addACLEntry(&aclResources[2], "Bob", READ | WRITE | EXECUTE);


    addACLEntry(&aclResources[3], "David", READ | EXECUTE);
    addACLEntry(&aclResources[3], "Eva", WRITE);
    addACLEntry(&aclResources[4], "Alice", READ | WRITE | EXECUTE);
    addACLEntry(&aclResources[4], "David", READ);


    CapabilityUser capabilityUsers[MAX_USERS];
    for (int i = 0; i < MAX_USERS; i++) {
        strcpy(capabilityUsers[i].user.name, users[i].name);
        capabilityUsers[i].capabilityCount = 0;
    }


    addCapability(&capabilityUsers[0], "File1", READ | WRITE);
    addCapability(&capabilityUsers[0], "File2", EXECUTE);
    addCapability(&capabilityUsers[0], "File5", READ | WRITE | EXECUTE);
    addCapability(&capabilityUsers[1], "File1", READ);
    addCapability(&capabilityUsers[1], "File3", READ | WRITE | EXECUTE);
    addCapability(&capabilityUsers[2], "File2", READ | WRITE);


    addCapability(&capabilityUsers[3], "File4", READ | EXECUTE);
    addCapability(&capabilityUsers[4], "File4", WRITE);
    addCapability(&capabilityUsers[3], "File5", READ);


   
    checkACLAccess(&aclResources[0], "Alice", READ);
    checkACLAccess(&aclResources[0], "Bob", WRITE);
    checkACLAccess(&aclResources[0], "Charlie", READ);
    checkCapabilityAccess(&capabilityUsers[0], "File1", WRITE);
    checkCapabilityAccess(&capabilityUsers[1], "File1", WRITE);
    checkCapabilityAccess(&capabilityUsers[2], "File2", READ);
   //new
    checkACLAccess(&aclResources[3], "David", READ);
    checkACLAccess(&aclResources[3], "Eva", READ);
    checkACLAccess(&aclResources[4], "Alice", EXECUTE);
    checkACLAccess(&aclResources[4], "David", WRITE);
    checkACLAccess(&aclResources[2], "Eva", READ);
    checkACLAccess(&aclResources[1], "David", READ);
   
    checkCapabilityAccess(&capabilityUsers[3], "File4", EXECUTE);
    checkCapabilityAccess(&capabilityUsers[4], "File4", WRITE);
    checkCapabilityAccess(&capabilityUsers[3], "File5", READ);
    checkCapabilityAccess(&capabilityUsers[0], "File5", EXECUTE);
    checkCapabilityAccess(&capabilityUsers[4], "File1", READ);
    checkCapabilityAccess(&capabilityUsers[2], "File4", READ);


    return 0;
}