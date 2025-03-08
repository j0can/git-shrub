# Git Shrub

A compact visualization tool for Git history that displays commits, branches, merges, and PRs with dates in a tree-like structure.

## Features

- Visual representation of Git commits with chronological layout
- Color-coded branches for easy tracking
- Special markers for different commit types:
  - Regular commits (●)
  - Merge commits (◆)
  - Pull Requests (◉ with PR number)
- Displays commit hash, subject, author, and date information
- Branch and tag labels
- Works as a native Git subcommand

## Installation

### Method 1: Using the Install Script (Recommended)

```bash
# Clone the repository
git clone https://github.com/j0can/git-shrub.git
cd git-shrub

# Run the installer
./install.sh
```

The installer will:
- Compile the program
- Allow you to choose where to install (system-wide or user-only)
- Update your PATH if necessary
- Provide usage instructions

### Method 2: Manual Installation

1. Compile the program:
```bash
gcc -o git-shrub shrub.c
```

2. Make it executable and move it to a directory in your PATH:
```bash
chmod +x git-shrub
cp git-shrub ~/.local/bin/  # or /usr/local/bin/ (requires sudo)
```

3. Make sure the directory is in your PATH:
```bash
# Add to ~/.bashrc or ~/.zshrc if needed
export PATH="$PATH:$HOME/.local/bin"
```

### Method 3: Using Make

```bash
# Clone the repository
git clone https://github.com/yourusername/git-shrub.git
cd git-shrub

# Build and install
make
make install  # May require sudo depending on installation location
```

## Usage

Simply navigate to any Git repository and run:

```bash
git shrub
```

The output will show a visual representation of your git history with all commits, branches, PRs, and dates formatted in a tree-like structure.

## Example Output

```
● abcd123 Initial commit (John Doe, 2023-03-01 10:00:00 +0000) [main]
    ● efgh456 Add feature X (Jane Smith, 2023-03-02 11:30:00 +0000)
    |   ● ijkl789 Fix bug in feature X (Jane Smith, 2023-03-03 09:15:00 +0000)
    |   |
    |   ◆ mnop012 Merge pull request #42 (John Doe, 2023-03-04 16:45:00 +0000)
    |
    ● qrst345 Update documentation (Alice Johnson, 2023-03-05 14:20:00 +0000) [develop]
```

## How It Works

Git Shrub works by:
1. Parsing `git log` output to extract commit information
2. Identifying regular commits, merge commits, and PRs
3. Assigning positions to branches and commits
4. Rendering the tree with ANSI colors and Unicode symbols

## Contributing

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [CONTRIBUTING.md](docs/CONTRIBUTING.md) for more details.

## License

This project is licensed under the Apache License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by git's built-in log visualization but focused on providing a more detailed view
- GitHub Copilot (big help)
