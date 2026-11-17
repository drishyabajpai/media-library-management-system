# Media Library Management System

**Project 40 — Media Library with Efficient Indexing & Search**  
**Language:** C (C11)

A command-line media catalog for storing and managing media metadata such as title, artist/creator, genre, release year, and file path.

## Features

- Add, edit, and delete media records
- Display all records
- Search by title keyword, artist/creator, or genre
- Hash-based indexes for title words, artist, and genre
- Linked-list storage and index buckets
- Sort records by title or year with paginated output
- Import records from a CSV file
- Save the library to `media_library.csv` and reload it on the next run
- Dynamic memory allocation for metadata and index nodes

## Requirements

- GCC or another C11-compatible compiler
- Windows, Linux, or macOS terminal

## Compile

### Windows (PowerShell / VS Code terminal)

```powershell
gcc -std=c11 -Wall -Wextra -pedantic media_library.c -o media_library.exe
```

### Linux / macOS

```bash
gcc -std=c11 -Wall -Wextra -pedantic media_library.c -o media_library
```

## Run

Windows:

```powershell
.\media_library.exe
```

Linux / macOS:

```bash
./media_library
```

## CSV format

The application uses this header:

```csv
id,title,artist,genre,year,path
```

A small example dataset is included in [`sample_media.csv`](sample_media.csv). To import it, choose **9. Import from CSV** and enter `sample_media.csv`.

The application automatically loads `media_library.csv` at startup when that file exists. Choose **10. Save library to CSV** to save manually, or **0. Save and exit** to save before closing.

## Data structures and concepts

| Concept | Use in this project |
|---|---|
| `struct Media` | Stores one media record and links records in a singly linked list |
| Singly linked list | Stores the library records dynamically |
| Hash table | Provides bucket-based indexes for title words, artist, and genre |
| Linked index nodes | Store pointers to media records in hash buckets |
| Dynamic memory | Allocates records and variable-length metadata at runtime |
| File I/O | Persists data using CSV import/export |
| Sorting | Uses the C standard library `qsort()` for title/year views |

## Project files

```text
media-library-management-system/
├── media_library.c
├── sample_media.csv
└── README.md
```

## Notes

- The program stores file paths as metadata; it does not play or open media files.
- `media_library.csv` is generated when the application saves the library. It is intentionally not committed as runtime user data.
- This is a console-based academic project demonstrating C programming, pointers, structures, linked lists, hashing, sorting, and file handling.
