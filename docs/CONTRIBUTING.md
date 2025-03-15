# Contributing to Git Shrub

Thank you for considering contributing to Git Shrub!


## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the existing issues list as you might find out that you don't need to create one. When you are creating a bug report, please include as many details as possible:

* Use a clear and descriptive title
* Describe the exact steps which reproduce the problem
* Provide specific examples to demonstrate the steps
* Describe the behavior you observed after following the steps
* Explain which behavior you expected to see instead and why
* Include details about your configuration and environment:
  * Which version of Git Shrub are you using?
  * What's the name and version of the OS you're using?
  * What's your Git version?

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

* Use a clear and descriptive title
* Provide a step-by-step description of the suggested enhancement
* Provide specific examples to demonstrate the steps
* Describe the current behavior and explain which behavior you expected to see instead
* Explain why this enhancement would be useful to most Git Shrub users

### Pull Requests

* Fill in the required template
* Do not include issue numbers in the PR title
* Follow the C coding style guide
* Include appropriate test cases
* Document new code based on the Documentation Styleguide
* End all files with a newline

## Development Process

1. Fork the repo and create your branch from `master`
2. If you've added code that should be tested, add tests
3. If you've changed APIs, update the documentation
4. Ensure the test suite passes
5. Make sure your code follows the style guidelines
6. Issue that pull request!

### Local Development

1. Clone your fork of the repo
```bash
git clone https://github.com/j0can/git-shrub
```

2. Create a branch for local development
```bash
git checkout -b name-of-your-bugfix-or-feature
```

3. Make your changes locally

4. Commit your changes
```bash
git add .
git commit -m "Your detailed description of your changes"
```

5. Push your branch
```bash
git push origin name-of-your-bugfix-or-feature
```

6. Submit a pull request through the GitHub website

## Style Guidelines

### Git Commit Messages

* Use the present tense ("Add feature" not "Added feature")
* Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
* Limit the first line to 72 characters or less
* Reference issues and pull requests liberally after the first line
* Consider starting the commit message with an applicable emoji:
    * 🎨 `:art:` when improving the format/structure of the code
    * 🐎 `:racehorse:` when improving performance
    * 📝 `:memo:` when writing docs
    * 🐛 `:bug:` when fixing a bug
    * 🔥 `:fire:` when removing code or files

### C Style Guide

* Use 4 spaces for indentation
* Maximum line length is 80 characters
* Use descriptive variable names
* Comment your code, but don't overdo it
* Function names should be in snake_case
* Constants should be in UPPER_CASE
* Always use braces for control structures, even for single-line blocks

## Documentation Styleguide

* Use Markdown
* Reference functions with backticks: \`function_name()\`
* Provide clear, concise examples
* Keep line length to a maximum of 80 characters
* Use headers appropriately (# for main title, ## for sections, etc.)

## Additional Notes

### Issue and Pull Request Labels

* `bug` - Something isn't working
* `enhancement` - New feature or request
* `documentation` - Improvements or additions to documentation
* `good first issue` - Good for newcomers
* `help wanted` - Extra attention is needed
* `invalid` - Something's wrong
* `question` - Further information is requested
* `wontfix` - This will not be worked on 