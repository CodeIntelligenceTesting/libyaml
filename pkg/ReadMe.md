# How to Make a `libci` Release

## Versioning

Update libci version in:
* announcement.msg
* Changes
* CMakeLists.txt
  * `CI_VERSION_MAJOR`, `CI_VERSION_MINOR`, `CI_VERSION_PATCH`
* .appveyor.yml
* configure.ac
  * `CI_MAJOR`, `CI_MINOR`, `CI_PATCH`, `CI_RELEASE`, `CI_CURRENT`, `CI_REVISION`

Commit and push everything to `release/0.x.y`.

## Test and Create Release Archives

### GitHub Actions Automation

The github workflow:

    .github/workflows/dist.ci

will do this automatically for you.

#### .github/workflows/dist.ci

This workflow will create release archives (`tar.gz` and `zip`).

### Manually

Make sure you have a clean git repository (no changed files).
The following process will clone your current git directory.

This will need the docker image `ciio/libci-dev`.
You can either pull it, or create it yourself:

    make docker-build

### Create dist archives

Run:

    make docker-dist

It will run `make dist` in the container to create a tarball written to
`pkg/docker/output`.
It will also create a zipfile.

## Update master

    git merge release/0.x.y
    git tag -a 0.x.y
    # <Editor opens>
    # Paste the corresponding entry from the Changes file
    # Look at an earlier release for how it should look like:
    #   git show 0.2.3
    git push origin master 0.x.y

## Create a GitHub release

Go to "Releases" and click on "Draft a new release".

Fill in the tag you just created in the previous step.

Fill in the release title: v0.x.y

Paste the changelog into the description field.

Upload the tar.gz and .zip file.

You can "Save draft" and publish later, or directly click on "Publish release".

## Update pyci.org

See <https://github.com/ci/pyci.org/blob/master/ReadMe.md>.

