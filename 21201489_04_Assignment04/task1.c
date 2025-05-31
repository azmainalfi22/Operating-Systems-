#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 5
#define MAX_RESOURCES 5
#define MAX_NAME_LEN 20

// Permission Enum
typedef enum {
    READ = 1,
    WRITE = 2,
    EXECUTE = 4
} Permission;

// User and Resource Definitions
typedef struct {
    char name[MAX_NAME_LEN];
} User;

typedef struct {
    char name[MAX_NAME_LEN];
} Resource;

// ACL Entry
typedef struct {
    char userName[MAX_NAME_LEN];
    int permissions;
} ACLEntry;

typedef struct {
    char resourceName[MAX_NAME_LEN];
    ACLEntry aclEntries[MAX_USERS];
    int aclCount;
} ACLControlledResource;

// Capability Entry
typedef struct {
    char resourceName[MAX_NAME_LEN];
    int permissions;
} Capability;

typedef struct {
    char userName[MAX_NAME_LEN];
    Capability capabilities[MAX_RESOURCES];
    int capabilityCount;
} CapabilityUser;

// Utility Functions
void printPermissions(int perm) {
    if (perm & READ) printf("Read ");
    if (perm & WRITE) printf("Write ");
    if (perm & EXECUTE) printf("Execute ");
}

int hasPermission(int userPerm, int requiredPerm) {
    return (userPerm & requiredPerm) == requiredPerm;
}

// ACL System
void checkACLAccess(ACLControlledResource *res, const char *userName, int perm) {
    for (int i = 0; i < res->aclCount; i++) {
        if (strcmp(res->aclEntries[i].userName, userName) == 0) {
            printf("ACL Check: User %s requests ", userName);
            printPermissions(perm);
            printf("on %s: %s\n", res->resourceName,
                hasPermission(res->aclEntries[i].permissions, perm) ? "Access GRANTED" : "Access DENIED");
            return;
        }
    }
    printf("ACL Check: User %s has NO entry for resource %s: Access DENIED\n", userName, res->resourceName);
}

// Capability System
void checkCapabilityAccess(CapabilityUser *user, const char *resourceName, int perm) {
    for (int i = 0; i < user->capabilityCount; i++) {
        if (strcmp(user->capabilities[i].resourceName, resourceName) == 0) {
            printf("Capability Check: User %s requests ", user->userName);
            printPermissions(perm);
            printf("on %s: %s\n", resourceName,
                hasPermission(user->capabilities[i].permissions, perm) ? "Access GRANTED" : "Access DENIED");
            return;
        }
    }
    printf("Capability Check: User %s has NO capability for resource %s: Access DENIED\n", user->userName, resourceName);
}

int main() {
    // Users and resources
    User users[MAX_USERS] = {{"Alice"}, {"Bob"}, {"Charlie"}, {"Alfi"}, {"Azmain"}};
    Resource resources[MAX_RESOURCES] = {{"File1"}, {"File2"}, {"File3"}, {"File4"}, {"File5"}};

    // ACL Setup
    ACLControlledResource aclResources[MAX_RESOURCES] = {
        {"File1", {{"Alice", READ | WRITE}, {"Bob", READ}}, 2},
        {"File2", {{"Charlie", READ | EXECUTE}, {"Alice", READ}}, 2},
        {"File3", {{"Bob", WRITE}, {"Alfi", READ | WRITE}}, 2},
        {"File4", {{"Azmain", READ | EXECUTE}}, 1},
        {"File5", {{"Alice", READ | WRITE | EXECUTE}}, 1}
    };

    // Capability Setup
    CapabilityUser capabilityUsers[MAX_USERS] = {
        {"Alice", {{"File1", READ | WRITE}, {"File2", READ}, {"File5", READ | WRITE | EXECUTE}}, 3},
        {"Bob", {{"File1", READ}, {"File3", WRITE}}, 2},
        {"Charlie", {{"File2", READ | EXECUTE}}, 1},
        {"Alfi", {{"File3", READ | WRITE}}, 1},
        {"Azmain", {{"File4", READ | EXECUTE}}, 1}
    };

    // ACL Access Checks
    checkACLAccess(&aclResources[0], "Charlie", READ);     // DENIED
    checkACLAccess(&aclResources[1], "Alice", EXECUTE);     // DENIED
    checkACLAccess(&aclResources[2], "Bob", WRITE);         // GRANTED
    checkACLAccess(&aclResources[3], "Alfi", READ);        // DENIED
    checkACLAccess(&aclResources[4], "Azmain", EXECUTE);       // DENIED
    checkACLAccess(&aclResources[4], "Alice", READ);        // GRANTED

    // Capability Access Checks
    checkCapabilityAccess(&capabilityUsers[0], "File2", WRITE);   // DENIED
    checkCapabilityAccess(&capabilityUsers[1], "File1", READ);    // GRANTED
    checkCapabilityAccess(&capabilityUsers[2], "File3", EXECUTE); // DENIED
    checkCapabilityAccess(&capabilityUsers[3], "File4", READ);    // DENIED
    checkCapabilityAccess(&capabilityUsers[4], "File5", EXECUTE); // DENIED
    checkCapabilityAccess(&capabilityUsers[0], "File5", EXECUTE); // GRANTED

    return 0;
}
