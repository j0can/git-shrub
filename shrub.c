#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_LINE_LENGTH 4096
#define MAX_BRANCHES 100
#define MAX_COMMITS 10000
#define DATE_LENGTH 30
#define DEBUG 0

typedef struct {
    char hash[41];
    char short_hash[8];
    char subject[256];
    char author[128];
    char date[DATE_LENGTH];
    time_t timestamp;
    char refs[256];
    int is_merge;
    int is_pr;
    char pr_number[16];
    int x_pos;
    int y_pos;
    char parent_hashes[5][41];
    int parent_count;
} Commit;

typedef struct {
    char name[128];
    char hash[41];
    int color;
    int x_pos;
} Branch;

Commit commits[MAX_COMMITS];
Branch branches[MAX_BRANCHES];
int commit_count = 0;
int branch_count = 0;

// ANSI color codes for branches
const char* colors[] = {
    "\033[1;31m", // Red
    "\033[1;32m", // Green
    "\033[1;33m", // Yellow
    "\033[1;34m", // Blue
    "\033[1;35m", // Magenta
    "\033[1;36m", // Cyan
    "\033[1;91m", // Bright Red
    "\033[1;92m", // Bright Green
    "\033[1;93m", // Bright Yellow
    "\033[1;94m", // Bright Blue
    "\033[1;95m", // Bright Magenta
    "\033[1;96m"  // Bright Cyan
};
#define COLOR_COUNT 12
#define RESET_COLOR "\033[0m"

// Function to execute a command and return the output
char* execute_command(const char* command) {
    FILE* fp;
    static char buffer[MAX_LINE_LENGTH * 100];
    char line[MAX_LINE_LENGTH];
    
    buffer[0] = '\0';
    
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command: %s\n", command);
        return buffer;
    }
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        strcat(buffer, line);
    }
    
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "Command exited with status %d: %s\n", status, command);
    }
    
    return buffer;
}


// Function to parse git log and fill the commits array
void parse_git_log() {
    char command[MAX_COMMAND_LENGTH];
    char *log_output;
    char *line, *next_line;
    
    // Modify the git log format to be more reliable
    // Use a custom separator that's unlikely to appear in commit messages
    snprintf(command, MAX_COMMAND_LENGTH, 
             "git log --all --date=iso --pretty=format:\"COMMIT_SEP%%H|%%h|%%s|%%an|%%ad|%%P|%%D\" --graph");
    
    if (DEBUG) {
    printf("Executing: %s\n", command);  // Debug output 
    }
    
    log_output = execute_command(command);
    
    // Check if we got any output
    if (log_output == NULL || strlen(log_output) < 10) {
        printf("Error: Failed to get git log output\n");
        return;
    }
    
    line = strtok_r(log_output, "\n", &next_line);
    while (line != NULL && commit_count < MAX_COMMITS) {
        if (strstr(line, "COMMIT_SEP") != NULL) {
            char *commit_part = strstr(line, "COMMIT_SEP");
            if (commit_part) {
                commit_part += 10; // Skip "COMMIT_SEP"
                
                // Parse the fields
                char *hash = strtok(commit_part, "|");
                if (hash) {
                    strncpy(commits[commit_count].hash, hash, 40);
                    commits[commit_count].hash[40] = '\0';
                    
                    char *short_hash = strtok(NULL, "|");
                    if (short_hash) {
                        strncpy(commits[commit_count].short_hash, short_hash, 7);
                        commits[commit_count].short_hash[7] = '\0';
                    }
                    
                    char *subject = strtok(NULL, "|");
                    if (subject) {
                        strncpy(commits[commit_count].subject, subject, 255);
                        commits[commit_count].subject[255] = '\0';
                        
                        // Check if it's a PR
                        if (strstr(subject, "Merge pull request") || 
                            strstr(subject, "Merge PR") || 
                            strstr(subject, "Pull request")) {
                            commits[commit_count].is_pr = 1;
                            
                            // Extract PR number
                            char *pr_start = strstr(subject, "#");
                            if (pr_start) {
                                char pr_num[16] = {0};
                                int i = 0;
                                pr_start++; // Skip the '#'
                                while (*pr_start && *pr_start != ' ' && *pr_start != ')' && i < 15) {
                                    pr_num[i++] = *pr_start++;
                                }
                                pr_num[i] = '\0';
                                strcpy(commits[commit_count].pr_number, pr_num);
                            }
                        } else {
                            commits[commit_count].is_pr = 0;
                            commits[commit_count].pr_number[0] = '\0';
                        }
                    }
                    
                    char *author = strtok(NULL, "|");
                    if (author) {
                        strncpy(commits[commit_count].author, author, 127);
                        commits[commit_count].author[127] = '\0';
                    }
                    
                    char *date = strtok(NULL, "|");
                    if (date) {
                        strncpy(commits[commit_count].date, date, DATE_LENGTH-1);
                        commits[commit_count].date[DATE_LENGTH-1] = '\0';
                        
                        // Convert to timestamp for sorting
                        struct tm tm = {0};
                        if (strptime(date, "%Y-%m-%d %H:%M:%S %z", &tm) != NULL) {
                            commits[commit_count].timestamp = mktime(&tm);
                        } else {
                            // Fallback if date parsing fails
                            commits[commit_count].timestamp = time(NULL);
                        }
                    }
                    
                    char *parents = strtok(NULL, "|");
                    if (parents) {
                        commits[commit_count].parent_count = 0;
                        if (strlen(parents) > 0) {
                            char *parent_token;
                            char parents_copy[256];
                            strncpy(parents_copy, parents, 255);
                            parents_copy[255] = '\0';
                            
                            parent_token = strtok(parents_copy, " ");
                            while (parent_token != NULL && commits[commit_count].parent_count < 5) {
                                strncpy(commits[commit_count].parent_hashes[commits[commit_count].parent_count], 
                                        parent_token, 40);
                                commits[commit_count].parent_hashes[commits[commit_count].parent_count][40] = '\0';
                                commits[commit_count].parent_count++;
                                parent_token = strtok(NULL, " ");
                            }
                        }
                        
                        // If it has more than one parent, it's a merge commit
                        if (commits[commit_count].parent_count > 1) {
                            commits[commit_count].is_merge = 1;
                        } else {
                            commits[commit_count].is_merge = 0;
                        }
                    }
                    
                    char *refs = strtok(NULL, "|");
                    if (refs) {
                        if (refs && strlen(refs) > 0) {
                            strncpy(commits[commit_count].refs, refs, 255);
                            commits[commit_count].refs[255] = '\0';
                            
                            // Extract branch names
                            if (strstr(refs, "HEAD -> ")) {
                                char *branch_start = strstr(refs, "HEAD -> ");
                                branch_start += 8; // Skip "HEAD -> "
                                
                                if (branch_count < MAX_BRANCHES) {
                                    int i = 0;
                                    while (branch_start[i] != ',' && branch_start[i] != '\0' && branch_start[i] != ' ' && i < 127) {
                                        branches[branch_count].name[i] = branch_start[i];
                                        i++;
                                    }
                                    branches[branch_count].name[i] = '\0';
                                    strcpy(branches[branch_count].hash, commits[commit_count].hash);
                                    branches[branch_count].color = branch_count % COLOR_COUNT;
                                    branch_count++;
                                }
                            }
                            
                            // Also check for refs/heads/ branches
                            char *branch_ref = refs;
                            while ((branch_ref = strstr(branch_ref, "refs/heads/")) != NULL) {
                                branch_ref += 11; // Skip "refs/heads/"
                                
                                if (branch_count < MAX_BRANCHES) {
                                    int i = 0;
                                    while (branch_ref[i] != ',' && branch_ref[i] != '\0' && branch_ref[i] != ' ' && i < 127) {
                                        branches[branch_count].name[i] = branch_ref[i];
                                        i++;
                                    }
                                    branches[branch_count].name[i] = '\0';
                                    strcpy(branches[branch_count].hash, commits[commit_count].hash);
                                    branches[branch_count].color = branch_count % COLOR_COUNT;
                                    branch_count++;
                                }
                                
                                branch_ref++;
                            }
                        } else {
                            commits[commit_count].refs[0] = '\0';
                        }
                    }
                    
                    commit_count++;
                }
            }
        }
        
        line = strtok_r(NULL, "\n", &next_line);
    }
    
    // Add debug output
    if (commit_count == 0) {
        printf("No commits were parsed. Debug info:\n");
        printf("Git command output length: %zu\n", strlen(log_output));
        printf("First 100 chars of output: %.100s\n", log_output);
    } else {
    if (DEBUG) {
    fprintf(stderr, "Successfully parsed %d commits and %d branches\n", commit_count, branch_count);
    }
    }
}

// Assign horizontal positions to branches
void assign_branch_positions() {
    int max_x = 0;
    
    // First, find the main branch (usually master or main)
    for (int i = 0; i < branch_count; i++) {
        if (strcmp(branches[i].name, "master") == 0 || 
            strcmp(branches[i].name, "main") == 0) {
            branches[i].x_pos = 0;
            max_x = 1;
            break;
        }
    }
    
    // Assign positions to other branches
    for (int i = 0; i < branch_count; i++) {
        if (branches[i].x_pos == 0 && 
            strcmp(branches[i].name, "master") != 0 && 
            strcmp(branches[i].name, "main") != 0) {
            branches[i].x_pos = max_x++;
        }
    }
}

// Assign positions to commits
void assign_commit_positions() {
    // Sort commits by timestamp (newest first)
    for (int i = 0; i < commit_count - 1; i++) {
        for (int j = 0; j < commit_count - i - 1; j++) {
            if (commits[j].timestamp < commits[j + 1].timestamp) {
                Commit temp = commits[j];
                commits[j] = commits[j + 1];
                commits[j + 1] = temp;
            }
        }
    }
    
    // Assign y positions (newest at top)
    for (int i = 0; i < commit_count; i++) {
        commits[i].y_pos = i;
        
        // Assign x position based on branch
        commits[i].x_pos = 0; // Default to leftmost position
        
        // Find the branch this commit belongs to
        for (int j = 0; j < branch_count; j++) {
            if (strcmp(commits[i].hash, branches[j].hash) == 0) {
                commits[i].x_pos = branches[j].x_pos;
                break;
            }
        }
        
        // For merge commits, try to position to the right
        if (commits[i].is_merge && commits[i].parent_count > 1) {
            int rightmost_parent = 0;
            
            // Find the rightmost parent branch
            for (int j = 0; j < commits[i].parent_count; j++) {
                for (int k = 0; k < commit_count; k++) {
                    if (strcmp(commits[i].parent_hashes[j], commits[k].hash) == 0) {
                        if (commits[k].x_pos > rightmost_parent) {
                            rightmost_parent = commits[k].x_pos;
                        }
                        break;
                    }
                }
            }
            
            // Position this merge commit on the rightmost parent's branch
            if (rightmost_parent > 0) {
                commits[i].x_pos = rightmost_parent;
            }
        }
    }
}

// Print the commit tree
void print_commit_tree() {
    int max_x = 0;
    
    // Find the maximum x position
    for (int i = 0; i < commit_count; i++) {
        if (commits[i].x_pos > max_x) {
            max_x = commits[i].x_pos;
        }
    }
    
    // Print the tree
    int y = 0;
    while (y < commit_count) {
        // Print a row of the tree
        char line[1024] = {0};
        
        // Find commit at this y position
        for (int i = 0; i < commit_count; i++) {
            if (commits[i].y_pos == y) {
                // Get branch color
                int color_index = 0;
                for (int j = 0; j < branch_count; j++) {
                    if (strstr(commits[i].refs, branches[j].name) != NULL) {
                        color_index = branches[j].color;
                        break;
                    }
                }
                
                // Pad to x position
                while (strlen(line) < commits[i].x_pos * 4) {
                    strcat(line, "    ");
                }
                
                // Add commit representation
                char commit_str[512];
                if (commits[i].is_merge) {
                    sprintf(commit_str, "%s◆ %s%s ", colors[color_index], commits[i].short_hash, RESET_COLOR);
                } else if (commits[i].is_pr) {
                    sprintf(commit_str, "%s◉ %s (PR #%s)%s ", colors[color_index], 
                            commits[i].short_hash, commits[i].pr_number, RESET_COLOR);
                } else {
                    sprintf(commit_str, "%s● %s%s ", colors[color_index], commits[i].short_hash, RESET_COLOR);
                }
                strcat(line, commit_str);
                
                // Add commit details
                char details[512];
                // Truncate subject if too long
                char subject[64];
                strncpy(subject, commits[i].subject, 63);
                subject[63] = '\0';
                if (strlen(commits[i].subject) > 63) {
                    strcat(subject, "...");
                }
                
                sprintf(details, "%s (%s, %s)", subject, commits[i].author, commits[i].date);
                strcat(line, details);
                
                // Add branch labels if any
                if (strlen(commits[i].refs) > 0) {
                    char *ref_token;
                    char refs_copy[256];
                    strcpy(refs_copy, commits[i].refs);
                    ref_token = strtok(refs_copy, ",");
                    
                    char branch_str[256] = {0};
                    strcat(branch_str, " [");
                    
                    while (ref_token != NULL) {
                        // Clean up ref name
                        char *ref_name = ref_token;
                        while (*ref_name == ' ') ref_name++;
                        
                        if (strstr(ref_name, "refs/heads/") != NULL) {
                            ref_name += 11; // Skip "refs/heads/"
                            strcat(branch_str, ref_name);
                            strcat(branch_str, ", ");
                        } else if (strstr(ref_name, "refs/tags/") != NULL) {
                            ref_name += 10; // Skip "refs/tags/"
                            strcat(branch_str, "tag: ");
                            strcat(branch_str, ref_name);
                            strcat(branch_str, ", ");
                        } else if (strstr(ref_name, "refs/remotes/") != NULL) {
                            ref_name += 13; // Skip "refs/remotes/"
                            strcat(branch_str, "remote: ");
                            strcat(branch_str, ref_name);
                            strcat(branch_str, ", ");
                        } else if (strlen(ref_name) > 0) {
                            strcat(branch_str, ref_name);
                            strcat(branch_str, ", ");
                        }
                        
                        ref_token = strtok(NULL, ",");
                    }
                    
                    // Remove trailing comma and space
                    int len = strlen(branch_str);
                    if (len > 2 && branch_str[len-2] == ',' && branch_str[len-1] == ' ') {
                        branch_str[len-2] = ']';
                        branch_str[len-1] = '\0';
                    } else {
                        strcat(branch_str, "]");
                    }
                    
                    strcat(line, branch_str);
                }
                
                break;
            }
        }
        
        // Print the line
        if (strlen(line) > 0) {
            printf("%s\n", line);
        }
        
        y++;
    }
}

int main(int argc, char *argv[]) {
    // Check if git repository
    char *git_dir = execute_command("git rev-parse --git-dir 2>/dev/null");
    if (strlen(git_dir) == 0 || strstr(git_dir, "fatal:") != NULL) {
        fprintf(stderr, "Error: Not a git repository\n");
        return EXIT_FAILURE;
    }
    
    // Check if repo has any commits
    char *commit_count_str = execute_command("git rev-list --count HEAD 2>/dev/null");
    if (strlen(commit_count_str) == 0 || strstr(commit_count_str, "fatal:") != NULL || atoi(commit_count_str) == 0) {
        fprintf(stderr, "Error: This repository has no commits\n");
        return EXIT_FAILURE;
    }
    
    parse_git_log();
    
    if (commit_count > 0) {
        assign_branch_positions();
        assign_commit_positions();
        print_commit_tree();
    }
    
    return EXIT_SUCCESS;
}
