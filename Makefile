# VST Runner Makefile
# Common development tasks

.PHONY: help setup build build-release test benchmark format clean docker-build docker-test install

# Default target
.DEFAULT_GOAL := help

# Detect number of CPU cores
NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Build directory
BUILD_DIR := build
BUILD_TYPE ?= Debug

help: ## Show this help message
	@echo "VST Runner - Development Tasks"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'

setup: ## Run initial development environment setup
	@chmod +x setup.sh
	@./setup.sh

build: ## Build project (Debug)
	@echo "Building project (Debug)..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON
	@cmake --build $(BUILD_DIR) --config Debug -j$(NPROC)
	@echo "✅ Build complete"

build-release: ## Build project (Release)
	@echo "Building project (Release)..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON
	@cmake --build $(BUILD_DIR) --config Release -j$(NPROC)
	@echo "✅ Release build complete"

test: ## Run unit tests
	@echo "Running tests..."
	@if [ -f "$(BUILD_DIR)/VSTRunnerTests_artefacts/Debug/VSTRunnerTests" ]; then \
		$(BUILD_DIR)/VSTRunnerTests_artefacts/Debug/VSTRunnerTests; \
	elif [ -f "$(BUILD_DIR)/VSTRunnerTests_artefacts/Release/VSTRunnerTests" ]; then \
		$(BUILD_DIR)/VSTRunnerTests_artefacts/Release/VSTRunnerTests; \
	else \
		echo "❌ Tests not built. Run 'make build' first."; \
		exit 1; \
	fi

test-core: ## Run Core tests only
	@echo "Running Core tests..."
	@$(BUILD_DIR)/VSTRunnerTests_artefacts/Debug/VSTRunnerTests --category Core

test-web: ## Run Web tests only
	@echo "Running Web tests..."
	@$(BUILD_DIR)/VSTRunnerTests_artefacts/Debug/VSTRunnerTests --category Web

benchmark: ## Run performance benchmarks
	@echo "Running benchmarks..."
	@if [ -f "$(BUILD_DIR)/VSTRunnerBenchmarks_artefacts/Release/VSTRunnerBenchmarks" ]; then \
		$(BUILD_DIR)/VSTRunnerBenchmarks_artefacts/Release/VSTRunnerBenchmarks; \
	elif [ -f "$(BUILD_DIR)/VSTRunnerBenchmarks_artefacts/Debug/VSTRunnerBenchmarks" ]; then \
		$(BUILD_DIR)/VSTRunnerBenchmarks_artefacts/Debug/VSTRunnerBenchmarks; \
	else \
		echo "❌ Benchmarks not built. Run 'make build-release' first."; \
		exit 1; \
	fi

format: ## Format code with clang-format
	@echo "Formatting code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find Source -name '*.cpp' -o -name '*.h' | xargs clang-format -i; \
		echo "✅ Code formatted"; \
	else \
		echo "❌ clang-format not found. Install with:"; \
		echo "  Linux: sudo apt-get install clang-format"; \
		echo "  macOS: brew install clang-format"; \
		exit 1; \
	fi

format-check: ## Check code formatting without modifying files
	@echo "Checking code formatting..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find Source -name '*.cpp' -o -name '*.h' | xargs clang-format -n -Werror; \
		echo "✅ Code formatting OK"; \
	else \
		echo "❌ clang-format not found"; \
		exit 1; \
	fi

clean: ## Clean build directory
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "✅ Clean complete"

clean-all: clean ## Clean everything (build, test results, benchmarks)
	@echo "Cleaning test results and benchmarks..."
	@rm -rf coverage/ test_results/ benchmarks/results/
	@echo "✅ Deep clean complete"

docker-build: ## Build Docker image
	@echo "Building Docker image..."
	@docker build -t vst-runner .
	@echo "✅ Docker image built"

docker-test: ## Run tests in Docker container
	@echo "Running tests in Docker..."
	@docker run --rm vst-runner /vst-runner/build/VSTRunnerTests_artefacts/Release/VSTRunnerTests

docker-shell: ## Open shell in Docker container
	@echo "Opening Docker shell..."
	@docker run -it --rm vst-runner /bin/bash

install: ## Install to system (requires sudo)
	@echo "Installing VST Runner..."
	@cd $(BUILD_DIR) && sudo cmake --install . --config Release
	@echo "✅ Installation complete"

run: ## Run VST Runner (Debug)
	@echo "Running VST Runner..."
	@if [ -f "$(BUILD_DIR)/VSTRunner_artefacts/Debug/VSTRunner" ]; then \
		$(BUILD_DIR)/VSTRunner_artefacts/Debug/VSTRunner; \
	else \
		echo "❌ VST Runner not built. Run 'make build' first."; \
		exit 1; \
	fi

run-release: ## Run VST Runner (Release)
	@echo "Running VST Runner (Release)..."
	@if [ -f "$(BUILD_DIR)/VSTRunner_artefacts/Release/VSTRunner" ]; then \
		$(BUILD_DIR)/VSTRunner_artefacts/Release/VSTRunner; \
	else \
		echo "❌ VST Runner not built. Run 'make build-release' first."; \
		exit 1; \
	fi

ci: format-check build test ## Run CI checks (format, build, test)
	@echo "✅ All CI checks passed"

dev: build test ## Quick development cycle (build + test)
	@echo "✅ Development cycle complete"

all: build-release test benchmark ## Build everything and run all checks
	@echo "✅ Full build and test complete"

# Submodule management
submodule-update: ## Update Git submodules
	@echo "Updating submodules..."
	@git submodule update --init --recursive
	@echo "✅ Submodules updated"

submodule-pull: ## Pull latest changes for submodules
	@echo "Pulling submodule updates..."
	@git submodule update --remote --merge
	@echo "✅ Submodules updated to latest"

# Documentation
docs: ## Open documentation in browser
	@echo "Opening documentation..."
	@if command -v xdg-open >/dev/null 2>&1; then \
		xdg-open README.md; \
	elif command -v open >/dev/null 2>&1; then \
		open README.md; \
	else \
		echo "Please open README.md manually"; \
	fi

# Info targets
info: ## Show project information
	@echo "VST Runner - Project Information"
	@echo ""
	@echo "Version: 1.0.0"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Build type: $(BUILD_TYPE)"
	@echo "CPU cores: $(NPROC)"
	@echo ""
	@echo "Targets available:"
	@echo "  - VSTRunner (main application)"
	@echo "  - VSTRunnerTests (test suite)"
	@echo "  - VSTRunnerBenchmarks (performance benchmarks)"

version: ## Show version information
	@echo "VST Runner v1.0.0"
	@if command -v cmake >/dev/null 2>&1; then \
		echo "CMake: $$(cmake --version | head -n1)"; \
	fi
	@if command -v git >/dev/null 2>&1; then \
		echo "Git: $$(git --version)"; \
	fi
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "clang-format: $$(clang-format --version | head -n1)"; \
	fi
