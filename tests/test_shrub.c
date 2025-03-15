#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../shrub.c"

void test_execute_command() {
    char* result = execute_command("git --version");
    assert(result != NULL);
    assert(strstr(result, "git version") != NULL);
    printf("✓ execute_command test passed\n");
}

void test_git_repo_setup() {
    system("mkdir -p test_repo && cd test_repo && git init");
    system("cd test_repo && echo 'test' > test.txt && git add . && git commit -m 'Initial commit'");
    
    char* result = execute_command("cd test_repo && git log --oneline");
    assert(result != NULL);
    assert(strstr(result, "Initial commit") != NULL);
    printf("✓ git repo setup test passed\n");
}

void test_parse_git_log() {
    commit_count = 0;
    parse_git_log();
    assert(commit_count > 0);
    assert(strlen(commits[0].hash) == 40);  // Full hash length
    assert(strlen(commits[0].subject) > 0);
    printf("✓ parse_git_log test passed\n");
}

void cleanup() {
    system("rm -rf test_repo");
}

int main() {
    printf("Running tests...\n");
    
    test_execute_command();
    test_git_repo_setup();
    test_parse_git_log();
    
    cleanup();
    printf("All tests passed!\n");
    return 0;
}
