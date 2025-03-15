# git-shrub 🌳

A Git visualization tool that provides an enhanced view of your repository's commit history with additional useful Git commands.

## Features

- Beautiful commit tree visualization with:
  - Full commit hashes for precise reference
  - Colored branch lines for easy tracking
  - Special symbols for different commit types:
    - Regular commits (●)
    - Merge commits (◆)
    - Pull request merges (◉)
  - Full commit messages with proper indentation
  - Author and timestamp information
  - Branch labels and Git references
  - Interactive scrolling with pager support

- Additional Git commands:
  - Reset latest commit while preserving changes
  - View repository statistics
  - Examine specific commit changes
  - Track file modification history

## Installation

### For Users

1. Fork the repository by clicking the "Fork" button at the top right of the GitHub page
2. Clone your forked repository:
```bash
git clone https://github.com/YOUR_USERNAME/git-shrub.git
cd git-shrub
```

3. Run the installation script:
```bash
./install.sh
```

4. Choose installation option:
   - System-wide installation (requires sudo)
   - User-only installation (recommended)

5. Add to PATH if necessary (for user-only installation):
```bash
export PATH="$PATH:$HOME/.local/bin"
```

## Usage

### Basic Commit Tree Visualization

Simply run in any Git repository:
```bash
git shrub
```

This will display an enhanced commit tree with:
- Full commit hashes
- Branch structure and relationships
- Commit messages, authors, and dates
- Special indicators for merge commits and pull requests
- Branch labels and references

### Additional Commands

#### Reset Latest Commit
```bash
git shrub -reset latest
```
Removes the latest commit while preserving all changes in your working directory. Useful for:
- Modifying your last commit
- Combining changes with other updates
- Changing commit messages

#### View Repository Statistics
```bash
git shrub -stats
```
Shows detailed repository information:
- Total number of commits
- Commits per author
- Active development days
- File statistics
- Most modified files

#### Examine Commit Changes
```bash
git shrub -diff <commit-hash>
```
Displays detailed information about a specific commit:
- Changed files
- Line-by-line modifications
- Commit message and metadata

#### Track File History
```bash
git shrub -files <filename>
```
Shows the commit history for a specific file:
- All commits that modified the file
- Commit messages and authors
- Timestamps of modifications

## Contributing

We welcome contributions! Please follow these steps:

1. Fork the Repository
   - Click the "Fork" button at the top right of the GitHub page
   - This creates your own copy of the repository under your GitHub account
   - Note: Direct cloning and pushing to the main repository is not allowed

2. Set Up Your Fork
   - Clone your forked repository:
     ```bash
     git clone https://github.com/YOUR_USERNAME/git-shrub.git
     ```
   - Add the original repository as upstream:
     ```bash
     git remote add upstream https://github.com/j0can/git-shrub.git
     ```

3. Keep Your Fork Updated
   - Regularly sync your fork with the upstream:
     ```bash
     git fetch upstream
     git checkout main
     git merge upstream/main
     ```

4. Make Your Changes
   - Create a new branch for your feature:
     ```bash
     git checkout -b feature/your-feature-name
     ```
   - Make your changes and commit them
   - Push to your fork:
     ```bash
     git push origin feature/your-feature-name
     ```

5. Submit a Pull Request
   - Go to your fork on GitHub
   - Click "New Pull Request"
   - Select your feature branch
   - Describe your changes in detail
   - Note: All PRs must be reviewed and approved before merging

### Repository Access Rules

- The main repository is private and can only be accessed through forks
- Direct commits to the master branch are restricted
- All changes must be submitted through Pull Requests
- Pull Requests require review and approval
- Only repository administrators can merge approved Pull Requests

### Common Forking Issues and Solutions

1. Out-of-Date Fork
   - **Issue**: Your fork becomes outdated as the main repository evolves
   - **Solution**: Regularly sync your fork with upstream (see step 3 above)

2. Merge Conflicts
   - **Issue**: Changes in your fork conflict with upstream changes
   - **Solution**: Resolve conflicts locally after syncing with upstream

3. Multiple Pull Requests
   - **Issue**: Managing multiple feature branches
   - **Solution**: Create separate branches for each feature/fix

4. Branch Management
   - **Issue**: Working on wrong branch
   - **Solution**: Always create new feature branches from updated main

## License

[MIT License](LICENSE)
