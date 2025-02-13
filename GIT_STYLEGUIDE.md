# MCFG/2 Git Styleguide
## Branches
### Naming

All new branches added to the upstream should follow the "path" name scheme, e.g.
`[name/]topic/documentation`, `[name/]issues/54`.

Root names can be any single one of the following:

* `topic` – A branch used for working on a more broad topic
* `feature` – A branch dedicated to implementing a new feature
* `issues` – A branch dedicated to fixing an issue
    * For branches with this root name, the second part should always
      be the ID of the issue.

### Merging

Branches should only ever be merged via Pull/Merge Requests. Before merging, the
branch should be based on the most recent commit on the repositories default branch.
This should only be ensured by rebasing the branch.

Commits should never be squashed.

Additionally when conflicts arise whilst pulling, merging should be avoided in favor
of rebasing to keep the history more linear.

## Commit Messages

All new git commit messages should be formatted in the following way:
```
scope: message
```

### Scopes

The scope should describe the general area of what the commit changes,
if it applies to multiple areas (although this should be avoided) you can
append scopes using commas.

Some general scopes are:

* all
* mcfg
* mcfg_util
* mcfg_format
* shared
* parser
* docs
    * Use this for modifications to the README, [GIT_]STYLEGUIDE, etc.
      For adding/modifying source code documentation, use the appropriate
      scope for the file which was modified alongside the doxygen meta-scope.

There are also "meta" scopes, these are for areas outside of the actual implementation
or to narrow down what you have changed. Meta scopes should always come after any other scope.

Some meta scopes are:

* [refactor]
* [tests]
* [building]
* [doxygen]
