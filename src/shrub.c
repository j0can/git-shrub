#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define VERSION "0.0.1"
#define MAX_COMMAND_LENGTH 1024
#define MAX_LINE_LENGTH 4096
#define MAX_BRANCHES 100
#define MAX_COMMITS 10000
#define DATE_LENGTH 30
#define DEBUG 0

#define COMMIT_SYMBOL "●"
#define MERGE_SYMBOL "◆"
#define PR_SYMBOL    "◉"

// Define structs first
typedef struct {
    char name[128];
    char hash[41];
    int color;
    int x_pos;
} Branch;

typedef struct {
    char hash[41];
    char short_hash[8];
    char subject[256];
    char full_message[4096];  // Added to store full commit message
    char author[128];
    char date[DATE_LENGTH];
    time_t timestamp;
    char refs[256];
    int is_merge;
    char symbol[8];
    int is_pr;
    char pr_number[16];
    int branch_index;
    int x_pos;
    int y_pos;
    int parent_count;
    char parent_hashes[5][41];
} Commit;

// Then declare function prototypes
void determine_commit_type(Commit *commit);
void print_graph_lines(Commit *commit, Branch *branches);
void print_usage();
int handle_reset_latest();
int handle_stats();
int handle_diff(const char* commit_hash);
int handle_files(const char* filename);

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
    
    // Modified format to include full commit message
    snprintf(command, MAX_COMMAND_LENGTH, 
             "git log --all --graph --date=iso"
             " --pretty=format:\"COMMIT_SEP%%H|%%h|%%s|%%an|%%ad|%%P|%%D|%%B\""
             " --date-order --color=always");
    
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
                    
                    // Parse full commit message
                    char *full_message = strtok(NULL, "|");
                    if (full_message) {
                        strncpy(commits[commit_count].full_message, full_message, 4095);
                        commits[commit_count].full_message[4095] = '\0';
                    }
                    
                    determine_commit_type(&commits[commit_count]);
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
    
    // Create a buffer for the entire output
    char *output_buffer = malloc(MAX_COMMITS * MAX_LINE_LENGTH * 4);
    output_buffer[0] = '\0';
    
    // Print the tree
    int y = 0;
    while (y < commit_count) {
        char line[4096] = {0};
        int branch_lines[MAX_BRANCHES] = {0};
        int child_connections[MAX_BRANCHES][MAX_BRANCHES] = {0};
        
        // Mark which branches have commits at this level and below
        for (int i = 0; i < commit_count; i++) {
            if (commits[i].y_pos >= y) {
                // Mark the branch line
                branch_lines[commits[i].x_pos] = 1;
                
                // If this is a merge commit, mark the connection to its parents
                if (commits[i].parent_count > 1) {
                    for (int j = 0; j < commits[i].parent_count; j++) {
                        for (int k = 0; k < commit_count; k++) {
                            if (strcmp(commits[i].parent_hashes[j], commits[k].hash) == 0) {
                                child_connections[commits[i].x_pos][commits[k].x_pos] = 1;
                                child_connections[commits[k].x_pos][commits[i].x_pos] = 1;
                                break;
                            }
                        }
                    }
                }
                // For regular commits, mark connection to single parent
                else if (commits[i].parent_count == 1) {
                    for (int k = 0; k < commit_count; k++) {
                        if (strcmp(commits[i].parent_hashes[0], commits[k].hash) == 0) {
                            child_connections[commits[i].x_pos][commits[k].x_pos] = 1;
                            child_connections[commits[k].x_pos][commits[i].x_pos] = 1;
                            break;
                        }
                    }
                }
            }
        }
        
        // Find commit at this y position
        for (int i = 0; i < commit_count; i++) {
            if (commits[i].y_pos == y) {
                // Add initial indentation
                if (commits[i].x_pos > 0) {
                    strcat(line, "    ");  // Base indentation for non-root commits
                }
                
                // Print branch lines before commit
                for (int x = 1; x < commits[i].x_pos; x++) {
                    if (branch_lines[x] || child_connections[x][commits[i].x_pos]) {
                        strcat(line, "│   ");
                    } else {
                        strcat(line, "    ");
                    }
                }
                
                // Get branch color
                int color_index = 0;
                for (int j = 0; j < branch_count; j++) {
                    if (strstr(commits[i].refs, branches[j].name) != NULL) {
                        color_index = branches[j].color;
                        break;
                    }
                }
                
                // Add commit representation with hash
                char commit_str[1024];
                if (commits[i].is_merge) {
                    sprintf(commit_str, "%s◆ %s%s ", colors[color_index], commits[i].hash, RESET_COLOR);
                } else if (commits[i].is_pr) {
                    sprintf(commit_str, "%s◉ %s%s ", colors[color_index], commits[i].hash, RESET_COLOR);
                } else {
                    sprintf(commit_str, "%s● %s%s ", colors[color_index], commits[i].hash, RESET_COLOR);
                }
                strcat(line, commit_str);
                
                // Add commit details
                char details[2048];
                if (commits[i].is_pr) {
                    sprintf(details, "%s (PR #%s) (%s, %s)", 
                            commits[i].subject,
                            commits[i].pr_number,
                            commits[i].author,
                            commits[i].date);
                } else if (commits[i].is_merge) {
                    sprintf(details, "%s (Merge commit) (%s, %s)",
                            commits[i].subject,
                            commits[i].author,
                            commits[i].date);
                } else {
                    sprintf(details, "%s (%s, %s)",
                            commits[i].subject,
                            commits[i].author,
                            commits[i].date);
                }
                strcat(line, details);
                
                // Add branch labels if any
                if (strlen(commits[i].refs) > 0) {
                    strcat(line, " [");
                    char *ref_token = strtok(commits[i].refs, ",");
                    while (ref_token != NULL) {
                        while (*ref_token == ' ') ref_token++;
                        if (strstr(ref_token, "refs/heads/") != NULL) {
                            strcat(line, ref_token + 11);
                        } else if (strstr(ref_token, "HEAD -> ") != NULL) {
                            strcat(line, ref_token + 8);
                        } else {
                            strcat(line, ref_token);
                        }
                        ref_token = strtok(NULL, ",");
                        if (ref_token != NULL) strcat(line, ", ");
                    }
                    strcat(line, "]");
                }
                
                strcat(line, "\n");
                
                // Add full commit message if it exists and differs from subject
                if (strlen(commits[i].full_message) > strlen(commits[i].subject)) {
                    char *msg_ptr = commits[i].full_message;
                    char *first_newline = strchr(msg_ptr, '\n');
                    
                    if (first_newline && *(first_newline + 1) != '\0') {
                        // Skip the first line (subject) and any blank lines
                        msg_ptr = first_newline + 1;
                        while (*msg_ptr == '\n') msg_ptr++;
                        
                        if (*msg_ptr != '\0') {
                            // Add indentation for the message
                            char indent[128] = {0};
                            if (commits[i].x_pos > 0) {
                                strcat(indent, "    ");
                            }
                            for (int x = 1; x < commits[i].x_pos; x++) {
                                if (branch_lines[x] || child_connections[x][commits[i].x_pos]) {
                                    strcat(indent, "│   ");
                                } else {
                                    strcat(indent, "    ");
                                }
                            }
                            strcat(indent, "    ");  // Extra indent for message
                            
                            // Add each line of the message with proper indentation
                            char msg_line[1024];
                            while (*msg_ptr) {
                                char *newline = strchr(msg_ptr, '\n');
                                if (newline) {
                                    size_t len = newline - msg_ptr;
                                    strncpy(msg_line, msg_ptr, len);
                                    msg_line[len] = '\0';
                                    msg_ptr = newline + 1;
                                } else {
                                    strcpy(msg_line, msg_ptr);
                                    msg_ptr += strlen(msg_ptr);
                                }
                                
                                if (strlen(msg_line) > 0) {
                                    strcat(line, indent);
                                    strcat(line, msg_line);
                                    strcat(line, "\n");
                                }
                            }
                        }
                    }
                }
                
                break;
            }
        }
        
        // Add line to output buffer
        if (strlen(line) > 0) {
            strcat(output_buffer, line);
            
            // Add branch lines for visual connection
            if (y < commit_count - 1) {  // Don't add after last commit
                char connection_line[1024] = {0};
                int has_connections = 0;
                
                // Add initial indentation for non-root level
                if (commits[y+1].x_pos > 0) {
                    strcat(connection_line, "    ");
                }
                
                // Add branch lines
                for (int x = 1; x < max_x; x++) {
                    int is_connected = 0;
                    for (int j = 0; j < max_x; j++) {
                        if (child_connections[x][j] || child_connections[j][x]) {
                            is_connected = 1;
                            break;
                        }
                    }
                    
                    if (branch_lines[x] || is_connected) {
                        strcat(connection_line, "│   ");
                        has_connections = 1;
                    } else {
                        strcat(connection_line, "    ");
                    }
                }
                
                // Only add connection line if there are actual connections
                if (has_connections) {
                    strcat(output_buffer, connection_line);
                    strcat(output_buffer, "\n");
                }
            }
        }
        
        y++;
    }
    
    // Use less with proper options for git log-like experience
    FILE *pager = popen("less -R +G", "w");
    if (pager) {
        fputs(output_buffer, pager);
        pclose(pager);
    } else {
        printf("%s", output_buffer);
    }
    
    free(output_buffer);
}

void determine_commit_type(Commit *commit) {
    // Check for PR
    if (strstr(commit->subject, "Merge pull request #") != NULL) {
        strcpy(commit->symbol, PR_SYMBOL);
        // Extract PR number
        sscanf(strstr(commit->subject, "#"), "#%s", commit->pr_number);
        commit->is_pr = 1;
    }
    // Check for merge commit
    else if (strstr(commit->subject, "Merge") == commit->subject) {
        strcpy(commit->symbol, MERGE_SYMBOL);
        commit->is_merge = 1;
    }
    // Regular commit
    else {
        strcpy(commit->symbol, COMMIT_SYMBOL);
    }
}

void print_commit_line(Commit *commit, Branch *branches, int branch_count) {
    // Print graph connection lines
    print_graph_lines(commit, NULL);  // We don't use branches here
    
    // Get branch color from branch_index
    int color_index = (branch_count > 0 && commit->branch_index < branch_count) ? 
                     branches[commit->branch_index].color : 0;
    
    // Print commit symbol with color
    printf("%s%s%s ", colors[color_index], 
           commit->symbol, RESET_COLOR);
           
    // Print commit info
    printf("%s %.7s%s %s", 
           colors[color_index],
           commit->hash,
           RESET_COLOR,
           commit->subject);
           
    // Print PR number if applicable
    if (commit->is_pr) {
        printf(" (PR #%s)", commit->pr_number);
    }
    
    // Print author and date
    printf(" (%s, %s)\n", commit->author, commit->date);
}

// Add print_graph_lines implementation
void print_graph_lines(Commit *commit, Branch *branches) {
    (void)branches;  // Explicitly mark branches as unused
    for (int i = 0; i < commit->x_pos; i++) {
        printf("│ ");
    }
}

int handle_reset_latest() {
    // Get the latest commit hash
    char *latest_commit = execute_command("git rev-parse HEAD");
    if (strlen(latest_commit) == 0 || strstr(latest_commit, "fatal:") != NULL) {
        fprintf(stderr, "Error: Failed to get latest commit\n");
        return EXIT_FAILURE;
    }
    
    // Remove newline from commit hash
    latest_commit[strcspn(latest_commit, "\n")] = 0;
    
    // Create the reset command - using --soft to preserve changes
    char reset_cmd[MAX_COMMAND_LENGTH];
    snprintf(reset_cmd, MAX_COMMAND_LENGTH, "git reset --soft HEAD~1");
    
    // Execute the reset
    char *reset_result = execute_command(reset_cmd);
    if (strstr(reset_result, "fatal:") != NULL) {
        fprintf(stderr, "Error: Failed to reset to previous commit\n");
        return EXIT_FAILURE;
    }
    
    printf("Successfully unstaged commit: %s\n", latest_commit);
    printf("Your changes have been preserved and are ready to be committed again.\n");
    printf("You can now modify your changes and create a new commit using:\n");
    printf("  git add .\n");
    printf("  git commit -m \"Your new commit message\"\n");
    return EXIT_SUCCESS;
}

void print_usage() {
    printf("git-shrub version %s\n", VERSION);
    printf("Usage: git shrub [options]\n\n");
    printf("Options:\n");
    printf("  -reset latest         Unstage the latest commit (preserves changes)\n");
    printf("  -stats               Show repository statistics\n");
    printf("  -diff [commit]       Show changes in a specific commit\n");
    printf("  -files [filename]    Show commits that modified a specific file\n");
    printf("  -version             Show version information\n");
    printf("  (no options)         Display the commit tree\n");
}

int handle_stats() {
    char *output;
    char command[MAX_COMMAND_LENGTH];
    
    printf("\nRepository Statistics:\n");
    printf("====================\n\n");
    
    // Total number of commits
    output = execute_command("git rev-list --count HEAD");
    printf("Total commits: %s", output);
    
    // Commits per author
    printf("\nCommits per author:\n");
    snprintf(command, MAX_COMMAND_LENGTH, 
             "git shortlog -sn --all");
    output = execute_command(command);
    printf("%s", output);
    
    // Active days
    printf("\nRepository activity:\n");
    snprintf(command, MAX_COMMAND_LENGTH,
             "git log --format=%%ad --date=short | sort -u | wc -l");
    output = execute_command(command);
    printf("Active days: %s", output);
    
    // File statistics
    printf("\nFile statistics:\n");
    snprintf(command, MAX_COMMAND_LENGTH,
             "git ls-files | wc -l");
    output = execute_command(command);
    printf("Total files: %s", output);
    
    // Most modified files
    printf("\nMost modified files:\n");
    snprintf(command, MAX_COMMAND_LENGTH,
             "git log --pretty=format: --name-only | grep -v '^$' | sort | uniq -c | sort -rg | head -10");
    output = execute_command(command);
    printf("%s", output);
    
    return EXIT_SUCCESS;
}

int handle_diff(const char* commit_hash) {
    char command[MAX_COMMAND_LENGTH];
    char *output;
    
    // Verify commit hash exists
    snprintf(command, MAX_COMMAND_LENGTH, "git rev-parse --verify %s", commit_hash);
    output = execute_command(command);
    if (strstr(output, "fatal:") != NULL) {
        fprintf(stderr, "Error: Invalid commit hash\n");
        return EXIT_FAILURE;
    }
    
    // Show commit info
    snprintf(command, MAX_COMMAND_LENGTH,
             "git show --color=always %s", commit_hash);
    output = execute_command(command);
    
    // Use pager for output
    FILE *pager = popen("less -R", "w");
    if (pager) {
        fputs(output, pager);
        pclose(pager);
    } else {
        printf("%s", output);
    }
    
    return EXIT_SUCCESS;
}

int handle_files(const char* filename) {
    char command[MAX_COMMAND_LENGTH];
    char *output;
    
    // Show commits that modified the file
    snprintf(command, MAX_COMMAND_LENGTH,
             "git log --follow --pretty=format:'%%C(yellow)%%h%%Creset %%s (%%an, %%ad)' --date=iso -- %s",
             filename);
    output = execute_command(command);
    
    if (strlen(output) == 0) {
        fprintf(stderr, "Error: No commits found for file '%s'\n", filename);
        return EXIT_FAILURE;
    }
    
    printf("\nCommit history for file: %s\n", filename);
    printf("===============================\n\n");
    printf("%s\n", output);
    
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    // Check if git repository
    if (argc > 1 && strcmp(argv[1], "-version") == 0) {
        printf("git-shrub version %s\n", VERSION);
        return EXIT_SUCCESS;
    }
    
    char *git_dir = execute_command("git rev-parse --git-dir 2>/dev/null");
    if (strlen(git_dir) == 0 || strstr(git_dir, "fatal:") != NULL) {
        fprintf(stderr, "Error: Not a git repository\n");
        return EXIT_FAILURE;
    }
    
    // Handle command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "-reset") == 0) {
            if (argc != 3 || strcmp(argv[2], "latest") != 0) {
                print_usage();
                return EXIT_FAILURE;
            }
            return handle_reset_latest();
        }
        else if (strcmp(argv[1], "-stats") == 0) {
            return handle_stats();
        }
        else if (strcmp(argv[1], "-diff") == 0) {
            if (argc != 3) {
                fprintf(stderr, "Error: Please provide a commit hash\n");
                print_usage();
                return EXIT_FAILURE;
            }
            return handle_diff(argv[2]);
        }
        else if (strcmp(argv[1], "-files") == 0) {
            if (argc != 3) {
                fprintf(stderr, "Error: Please provide a filename\n");
                print_usage();
                return EXIT_FAILURE;
            }
            return handle_files(argv[2]);
        }
        else {
            print_usage();
            return EXIT_FAILURE;
        }
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
