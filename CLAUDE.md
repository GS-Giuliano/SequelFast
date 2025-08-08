# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SequelFast is an open-source MySQL and MariaDB client built with Qt 6.9.1 and C++17. It's a desktop application focused on speed and providing comprehensive database management features including connection management, SQL editing with syntax highlighting, data editing, backup/restore operations, user management, and schema browsing.

## Build System & Development Commands

This project uses **qmake** (Qt's build system) with a `.pro` file configuration.

### Core Build Commands
```bash
# Generate Makefile from .pro file
qmake SequelFast.pro

# Build the application
make

# Clean build artifacts
make clean

# Build for release
qmake CONFIG+=release
make
```

### Platform-Specific Deployment
```bash
# Linux deployment (creates distributable package)
./make_installer_linux.sh

# macOS Intel deployment
./make_installer_macos_intel.sh

# macOS Silicon deployment  
./make_installer_macos_silicon.sh
```

### Installation/Uninstallation (Linux)
```bash
# Install the application
./install_linux.sh

# Uninstall the application
./uninstall_linux.sh
```

## Architecture & Key Components

### Main Application Structure
- **MainWindow** (`src/mainwindow.h/cpp`) - Primary application window and orchestrator
- **Connection** (`src/connection.h/cpp`) - Database connection management dialog
- **Sql** (`src/sql.h/cpp`) - SQL editor with syntax highlighting and execution
- **Structure** (`src/structure.h/cpp`) - Table structure editor for schema modifications
- **Users** (`src/users.h/cpp`) - MySQL user management interface
- **Statistics** (`src/statistics.h/cpp`) - Database statistics and information display
- **Batch** (`src/batch.h/cpp`) - Batch operation processing

### Core Features & Classes
- **Backup/Restore** (`src/backup.h/cpp`, `src/restore.h/cpp`) - Database backup and restore operations
- **SqlHighlighter** (`src/sqlhighlighter.h/cpp`) - SQL syntax highlighting for code editor
- **TextEditCompleter** (`src/texteditcompleter.h/cpp`) - Auto-completion for SQL editor
- **TunnelSqlManager** (`src/tunnelsqlmanager.h/cpp`) - SSH tunnel management for secure connections
- **MacroInputDialog/MacroFormatDialog** - Macro system for parameterized queries

### Custom UI Components
- **TwoCheckboxDelegate** - Custom table cell renderer for dual checkbox states
- **ColoredItemDelegate** - Colored item rendering for connection lists
- Multiple regex-based delegates for table structure editing

### Resource Management
- **Theme System**: Light (`themes/light/`) and dark (`themes/dark/`) themes with complete QSS stylesheets
- **Icons**: Iconsax icon set in `resources/` directory
- **Internationalization**: Portuguese (Brazil) translation support (`SequelFast_pt_BR.ts`)

## Key Dependencies & Requirements

### Qt Modules Required
- Qt Core, GUI, Widgets (standard Qt desktop)
- Qt SQL (database connectivity) 
- Qt Network (for network operations)

### Database Drivers
The application requires MySQL/MariaDB SQL drivers. MariaDB drivers are recommended for better compatibility.

### External Dependencies
- OpenSSL for secure connections
- MySQL/MariaDB client libraries
- Platform-specific build tools (gcc/g++ for Linux, Xcode for macOS)

## Development Environment Setup

### Qt Installation Requirements
- Qt 6.9.1 or compatible version
- Qt Creator for development
- Platform-specific Qt components and build tools

### MySQL/MariaDB Driver Installation
See README.md for detailed platform-specific instructions for building and installing SQL drivers.

## Project Configuration

- **Language Standard**: C++17
- **Include Path**: `src/` directory is in include path
- **Resource Files**: Icons, themes, and UI forms compiled into executable
- **Deployment**: Self-contained packages with all required Qt libraries and dependencies

## Important Notes

- No existing CLAUDE.md file was found
- No test framework currently configured
- No linting tools currently configured in build system
- Application uses Qt's signal-slot system for UI event handling
- Macro system supports parameterized queries with various input types
- SSH tunneling supported for secure database connections
- Multi-platform deployment with platform-specific installers