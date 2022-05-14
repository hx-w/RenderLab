## NURBS Render System

[![CodeFactor](https://www.codefactor.io/repository/github/hx-w/nurbs/badge/master?s=9cf2a9b2a039e7494297917727036504ef3f21d4)](https://www.codefactor.io/repository/github/hx-w/nurbs/overview/master)

### Build and run

- **Win32**: `.\build.bat && .\main.exe`
- **MacOS**: `sh build.sh && ./main`

### Prepare

1. Move STL file to `./static/STL/`, and rename as `JawScan.stl`
2. Build and run

---

### Usage

- `W, A, S, D` for moving camera `front, left, back, right`
- `H` for hiding or showing nurbs face
- `T` for toggle shade mode: `GL_LINE, GL_POINT, GL_FILL`
- `Mouse pressing and moving` for rotating
- `Mouse scrolling` for scaling
- `Ctrl + Left click` for picking point in nurbs face-1
- `Ctrl + R` for clear picked ray/point
- `ESC` for exit

> **Tips**:
> Nurbs face will generate and show gradually,
> You will see those faces below initial camera position.

### Reference

- `src/render/libs`: open source libs for rendering
- `src/infrastructure`: adapted from actual projects
