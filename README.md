### Chronos — A Minimal Quantitative Trading Simulation Framework

Chronos is a hybrid C++/Python framework for building and testing low-latency trading systems. It simulates the full data path of a live market — from tick ingestion and order-book matching to strategy logic and analytics — while emphasizing performance, modularity, and realistic market mechanics.

The core engine is written in modern C++ to handle feed parsing, order matching, and concurrent message passing through lock-free queues and ZeroMQ sockets. On top of it, Python strategies subscribe to live market data, generate orders, and record fills, enabling rapid experimentation with statistical and algorithmic models.

Chronos provides an end-to-end workflow: replay historical or synthetic data, execute strategies in real time, collect latency and PnL metrics, and visualize results through Plotly dashboards. It’s designed as a compact but expressive showcase of quant-engineering principles — performance, determinism, and data transparency.
