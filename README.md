# **Order Exchange**

A multi-threaded order matching engine written in C++17. This project was built to understand how modern exchanges process orders, maintain order books, and match trades with low latency. The focus of the project is on the core matching engine, efficient data structures, and concurrent processing.

---
### **Features**
- Limit and Market orders
- Order Modify and Cancel
- Good Till Cancel (GTC)
- Immediate Or Cancel (IOC)
- Fill Or Kill (FOK)
- Price-Time Priority (FIFO)
- Multi-symbol support
- Symbol-based sharding
- Lock-free SPSC queue between dispatcher and matching engine

---
### **Architecture**

  <img width="1287" height="791" alt="project1" src="https://github.com/user-attachments/assets/aa9532d7-ef9c-4a6c-86d2-b2fc3f6e80f3" />


### Matching Engine Flow

  <img width="718" height="899" alt="Screenshot from 2026-07-24 00-31-43" src="https://github.com/user-attachments/assets/ab19359b-d398-407e-8e6d-aaf7655e0840" />

 
---
## **Benchmark**

##### **Environment**

**CPU** : AMD Ryzen 5 5500U

**Memory** : 8 GB

**OS** : Pop!_OS

**Compiler** : g++ (C++17, -O2)

**Benchmark Setup** :
- Synthetic workload
- Orders submitted directly to the matching engine
- Networking excluded
- Measures only the matching engine performance

### **Result**
**Orders Processed** : 2,000,000
**Elapsed Time** : 0.2461 s
**Throughput :** 8.12 Million Orders / Second
<img width="735" height="208" alt="Screenshot from 2026-07-24 00-11-25" src="https://github.com/user-attachments/assets/c79a4de1-9477-4f68-9a6b-f50dbeddb7a1" />

---
#### **Future Plans**
<img width="1287" height="1214" alt="Project2" src="https://github.com/user-attachments/assets/b68b7415-4231-444a-9bfd-ddedd8f5d311" />

- TCP networking
- Binary protocol improvements
- Event-driven networking using epoll
- Market data streaming
- Performance profiling and further optimization

---

#### **Build**

`g++ -O2 MatchingEngine/MatchingEngine.cpp OrderBook/OrderBook.cpp  Dispactcher/Dispatcher.cpp benchmark.cpp -I. -o order_exchange`

  ---

##### **Run:**

`./order_exchange`
