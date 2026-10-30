# host

NAS server written in modern C++.

## Requirements

- **Docker** (recommended), or locally:
  - GCC with **C++20** support
  - **CMake** ≥ 3.20
  - **rapidjson** (headers only)

## Build in Docker

```bash
./scripts/docker/build_image.sh   # build the host-dev-env image
./scripts/docker/build.sh         # run make inside the container
./scripts/docker/run.sh           # interactive shell, repo mounted at /workspace
```

`build.sh` forwards its arguments to `make`, e.g. `./scripts/docker/build.sh clean`.

The container runs as your user, so build output stays owned by you.

## Native build

| Target       | Description              |
|--------------|--------------------------|
| `make`       | Configure and build      |
| `make run`   | Run `build/host`         |
| `make clean` | Remove `build/`          |
