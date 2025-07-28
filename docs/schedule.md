# Master Plan: Concurrent Web Server Development Project (Enhanced with Reading Plan)

## Table of Contents
1. [Project Overview](#project-overview)
2. [Core Resources](#core-resources)
3. [Project Stages Overview](#project-stages-overview)
4. [Stage 1: Echo Server](#stage-1-echo-server)
5. [Stage 2: Sequential HTTP Server](#stage-2-sequential-http-server)
6. [Stage 3: Multi-threaded Server](#stage-3-multi-threaded-server)
7. [Stage 4: Event-driven Server](#stage-4-event-driven-server)
8. [Stage 5: Multi-process, Multi-core Server](#stage-5-multi-process-multi-core-server)
9. [Essential Debugging Skills](#essential-debugging-skills)
10. [Performance Testing Progression](#performance-testing-progression)
11. [Knowledge Checklists](#knowledge-checklists)
12. [Career Impact by Stage](#career-impact-by-stage)
13. [Adjustable Timeline](#adjustable-timeline)
14. [Measuring Success](#measuring-success)
15. [Decision Point After Stage 3](#decision-point-after-stage-3)

## Project Overview

**Goal**: Develop a professional-grade web server in C, progressing through increasingly sophisticated architectures to gain mastery of systems programming, networking, concurrency, and performance optimization.

**Project Stages**:
1. Echo Server (Network Foundations)
2. Sequential HTTP Server (Protocol Implementation)
3. Multi-threaded Server (Thread-based Concurrency)
4. Event-driven Server (Asynchronous I/O)
5. Multi-process, Multi-core Server (Process-based Parallelism)

**Career Impact**: This project will make you highly competitive for entry-level positions in systems engineering, backend development, infrastructure, and performance-focused roles.

## Core Resources

### Textbooks

1. **Computer Systems: A Programmer's Perspective (CSAPP)**
   - Primary reference for systems programming fundamentals
   - Key chapters: 10 (I/O), 11 (Networking), 12 (Concurrency)
   - Use for: Socket programming, concurrency mechanisms, memory management

2. **Computer Networking: A Top-Down Approach**
   - Comprehensive coverage of networking concepts at all layers
   - Key chapters: 1 (Introduction), 2 (Application Layer), 3 (Transport Layer), 6.6 (Data Center Networking)
   - Use for: Protocol understanding, HTTP details, TCP/IP concepts, networking architecture

3. **The Art of Debugging with GDB, DDD, and Eclipse**
   - Essential for developing debugging techniques
   - Key chapters: 1-4 (Basics), 5 (Multi-threaded debugging)
   - Use for: Systematic debugging approaches, debugging concurrent programs

4. **The Linux Programming Interface**
   - Advanced Linux programming concepts
   - Key chapters: 24-27 (Processes), 53-55 (IPC), 59-63 (Alternative I/O)
   - Use for: Process management, IPC mechanisms, event-driven programming

5. **UNIX Network Programming, Volume 1**
   - Classic text on network programming
   - Key chapters: 6 (I/O Multiplexing), 15 (IPC)
   - Use for: Event-driven architecture, process management patterns

6. **Systems Performance: Enterprise and the Cloud**
   - Performance analysis and optimization techniques
   - Key chapters: 5-6 (Applications, CPUs), 7-8 (Memory, Files), 13 (TCP)
   - Use for: Identifying bottlenecks, optimization strategies, benchmarking

### Online Resources

1. **POSIX Threads Programming Tutorial**
   - https://hpc-tutorials.llnl.gov/posix/
   - Guide to thread programming with practical examples
   - Use for: Thread creation, synchronization primitives, thread safety

2. **Operating Systems: Three Easy Pieces**
   - https://pages.cs.wisc.edu/~remzi/OSTEP/
   - Free online textbook with excellent concurrency chapters
   - Use for: Concurrency concepts, locks, condition variables, semaphores

3. **libuv Documentation**
   - https://docs.libuv.org/en/stable/
   - Node.js event loop library documentation
   - Use for: Event loop design patterns, asynchronous I/O techniques

4. **Nginx Architecture Documentation**
   - https://nginx.org/en/docs/
   - Real-world server architecture documentation
   - Use for: Worker process model, connection processing, performance tuning

5. **RFC 7230-7235: HTTP/1.1 Protocol**
   - https://datatracker.ietf.org/doc/html/rfc7230
   - Official HTTP protocol specification
   - Use for: HTTP message format details, header specifications, status codes

### Debugging Tools Documentation

1. **GDB Documentation**
   - https://sourceware.org/gdb/current/onlinedocs/gdb/
   - Comprehensive guide to GDB usage
   - Use for: Setting breakpoints, inspecting variables, stepping through code, debugging core dumps

2. **Valgrind User Manual**
   - https://valgrind.org/docs/
   - Memory debugging and leak detection
   - Use for: Finding memory leaks, uninitialized variables, invalid memory access

3. **Thread Sanitizer Documentation**
   - https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual
   - Data race detection tool
   - Use for: Finding race conditions, deadlocks, thread safety issues

4. **Performance Analysis Tools Manuals**
   - **perf** - https://perf.wiki.kernel.org/index.php/Main_Page
   - **strace** - https://man7.org/linux/man-pages/man1/strace.1.html
   - **tcpdump/Wireshark** - https://www.wireshark.org/docs/
   - Use for: CPU profiling, system call tracing, network packet analysis

### Reading Strategy for Technical Material

- Start with a complete read-through focusing on big ideas
- Re-read with specific implementation goals in mind
- Make notes connecting concepts to your code
- Review sections after implementing related features
- Document challenges and solutions with references to source material
   - Document challenges and solutions with references to source material

### Resource Priority by Stage

| Stage | Primary Resources | Supporting Resources |
|-------|------------------|---------------------|
| Stage 1 | CSAPP (Ch 10-11), Computer Networking (Ch 1, Ch 3.1-3.3), Art of Debugging (Ch 1-4) | GDB Documentation, strace manual, socket programming references |
| Stage 2 | CSAPP (Ch 11.5-11.6), Computer Networking (Ch 2.1-2.2, Ch 2.7), RFC 7230-7235 | Wireshark Documentation, HTTP protocol references |
| Stage 3 | CSAPP (Ch 12), Computer Networking (Ch 3.4-3.5), POSIX Threads Tutorial | Art of Debugging (Ch 5), OSTEP Concurrency Chapters |
| Stage 4 | Linux Programming Interface (Ch 59-63), Computer Networking (Ch 3.6-3.7), UNIX Network Programming (Ch 6) | libuv Documentation, Systems Performance (Ch 6-8) |
| Stage 5 | Linux Programming Interface (Ch 24-27, 53-55), Computer Networking (Ch 6.6), UNIX Network Programming (Ch 15) | Systems Performance (Ch 5-6, 13), Nginx Documentation |

## Project Stages Overview

| Stage | Focus | Duration | Impressiveness | Key Reading Topics |
|-------|-------|----------|---------------|-------------------|
| 1: Echo Server | Network foundations, Socket programming | 2 weeks | 4/10 | CSAPP (network basics), Computer Networking (Internet architecture, transport layer) |
| 2: Sequential HTTP Server | HTTP protocol, Static file serving | 3 weeks | 6/10 | Computer Networking (HTTP protocol, application layer), RFC 7230-7235 |
| 3: Multi-threaded Server | Thread programming, Synchronization | 4 weeks | 7.5/10 | CSAPP (concurrency), Computer Networking (TCP reliability, flow control) |
| 4: Event-driven Server | Non-blocking I/O, Event loops | 3-4 weeks | 8.5/10 | UNIX Network Programming (I/O multiplexing), Computer Networking (congestion control) |
| 5: Multi-process Server | Process management, IPC, Multi-core | 3-4 weeks | 9/10 | Linux Programming Interface (process management), Computer Networking (data center networking) |

## Stage 1: Echo Server

### Learning Focus
- Socket programming fundamentals
- Client-server architecture
- Basic network I/O
- Error handling and robustness
- Debugging networked applications

### Weekly Schedule

#### Week 1: Network Programming Foundations
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Socket API | Study socket API and TCP/IP basics | CSAPP Ch. 11, Computer Networking Ch 1.1-1.3 (Internet architecture and network edge/core) |
| 3-4 | Client Implementation | Implement basic client-socket functions | Computer Networking Ch 3.1-3.2 (Transport layer services and multiplexing) |
| 5-7 | Server Implementation | Implement basic server-socket functions and logging | Computer Networking Ch 3.3 (UDP - connectionless transport) |

#### Week 2: Robustness and Debugging
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Robust I/O | Study robust I/O patterns | CSAPP Ch. 10, Computer Networking Ch 1.4 (Delay, loss, throughput) |
| 3-4 | Error Handling | Implement robust read/write functions with error handling | Computer Networking Ch 1.5 (Protocol layers and encapsulation) |
| 5-6 | Debugging | Learn GDB basics and debug your implementation | Art of Debugging Ch 1-2 |
| 7 | Finalization | Add comprehensive error handling and logging | Review materials from week |

### Implementation Milestones
- [x] Basic socket communication between client and server
- [ ] Robust I/O functions handling partial reads/writes
- [ ] Error handling for all network operations
- [ ] Basic logging of connections and data transfers
- [ ] Clean shutdown and resource management

### Debugging Skills for This Stage
- Using GDB for basic debugging (breakpoints, stepping, variables)
- Using strace to monitor system calls
- Socket state inspection with netstat/ss
- Basic printf debugging strategies

### Practical Exercises
1. Deliberately break your echo server and practice finding the issues with GDB
2. Implement timeout handling for inactive connections
3. Test server behavior when clients disconnect unexpectedly
4. Use strace to observe all system calls your server makes

### Testing Tools
- netcat (nc) for manual client testing
- telnet for interactive testing
- wireshark/tcpdump for packet capture

## Stage 2: Sequential HTTP Server

### Learning Focus
- HTTP protocol implementation
- Request parsing and response generation
- MIME type handling
- Static file serving
- HTTP error handling
- Performance measurement

### Weekly Schedule

#### Week 3: HTTP Protocol Foundation
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | HTTP Basics | Study HTTP protocol basics | Computer Networking Ch 2.1 (Network application principles), CSAPP Ch 11.5 |
| 3-4 | Design | Design HTTP request parser and response generator | Computer Networking Ch 2.2.1-2.2.2 (HTTP overview, connections), RFC 7230 Sections 2-3 |
| 5-7 | Request Parsing | Implement basic HTTP request parsing | Computer Networking Ch 2.2.3 (HTTP message format) |

#### Week 4: Request Handling and Static Files
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Response Generation | Implement HTTP response generation | Computer Networking Ch 2.2.4 (User-server interaction, cookies) |
| 3-4 | File Serving | Add static file serving capabilities | Computer Networking Ch 2.2.5 (Web caching), CSAPP Ch 10 (file I/O) |
| 5-7 | MIME Types | Implement proper MIME type detection and handling | Computer Networking Ch 2.2.6 (HTTP/2), RFC 7231 Section 3.1.1 |

#### Week 5: Error Handling and Logging
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Error Responses | Implement proper HTTP error responses (404, 500, etc.) | RFC 7231 Section 6 (Response Status Codes) |
| 3-4 | Logging | Add comprehensive HTTP access and error logging | Review logging patterns from industry examples |
| 5-6 | Testing | Test with various browsers and HTTP clients | Computer Networking Ch 2.7 (Socket programming focus) |
| 7 | Performance | Performance testing and basic optimizations | Review Wireshark capture analysis techniques |

### Implementation Milestones
- [ ] HTTP GET request parsing
- [ ] HTTP response generation with proper headers
- [ ] Static file serving with correct MIME types
- [ ] Directory listing functionality
- [ ] Proper status code handling (200, 404, 403, 500, etc.)
- [ ] HTTP-specific logging (access log, error log)
- [ ] URL parameter handling

### Debugging Skills for This Stage
- HTTP protocol debugging with Wireshark
- HTTP request/response analysis
- File I/O performance debugging
- HTTP header validation

### Practical Exercises
1. Implement a simple HTML form processor (GET parameters)
2. Add caching headers to improve performance
3. Create a custom 404 page and other error responses
4. Benchmark your server with Apache Bench (ab)

### Testing Tools
- Browser developer tools
- curl for HTTP requests
- Apache Bench (ab) for basic load testing
- Wireshark for packet analysis

## Stage 3: Multi-threaded Server

### Learning Focus
- Thread programming fundamentals
- Synchronization mechanisms
- Thread pool patterns
- Race condition prevention
- Deadlock avoidance
- Performance with concurrency

### Weekly Schedule

#### Week 6: Threading Fundamentals
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Thread Basics | Study threading basics | CSAPP Ch. 12 (Introduction to Concurrency) |
| 3-4 | Thread Creation | Implement basic thread creation and management | POSIX Threads Tutorial Sections 1-3 |
| 5-7 | Synchronization | Practice with mutexes and condition variables | Computer Networking Ch 3.4 (Principles of reliable data transfer), CSAPP Ch 12.4-12.5 |

#### Week 7: Thread Safety
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Safety Analysis | Identify thread safety issues in your server | CSAPP Ch. 12.7 (Races), Art of Debugging Ch 5.1-5.2 |
| 3-4 | Safe Logging | Implement thread-safe logging | POSIX Threads Tutorial Section 4 (Synchronization) |
| 5-7 | Safe Handling | Make request handling thread-safe | Computer Networking Ch 3.5.1-3.5.3 (TCP connection, segment structure, RTT estimation) |

#### Week 8: Thread Pool Implementation
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Pool Design | Design thread pool architecture | OSTEP Concurrency Chapters 27-28 |
| 3-5 | Worker Threads | Implement worker thread pool | Computer Networking Ch 3.5.4-3.5.5 (TCP reliable data transfer, flow control) |
| 6-7 | Job Queue | Add job queue and work distribution | CSAPP Ch 12.5.1 (Producer-Consumer Problem) |

#### Week 9: Testing and Optimization
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Race Detection | Use Thread Sanitizer to identify race conditions | Thread Sanitizer documentation |
| 3-4 | Bug Fixing | Fix synchronization issues | Art of Debugging Ch 5.3-5.5 |
| 5-6 | Load Testing | Load test with concurrent connections | Computer Networking Ch 3.5.6 (TCP connection management) |
| 7 | Optimization | Performance tuning and optimization | Systems Performance Ch 6 (CPUs) |

### Implementation Milestones
- [ ] Thread pool for handling multiple connections
- [ ] Thread-safe request processing
- [ ] Synchronized access to shared resources
- [ ] Thread-safe logging mechanism
- [ ] Graceful thread shutdown
- [ ] Connection queuing during high load

### Debugging Skills for This Stage
- Thread Sanitizer for race condition detection
- GDB multi-threaded debugging
- Deadlock identification and resolution
- Thread state inspection

### Practical Exercises
1. Implement the producer-consumer pattern with your connection queue
2. Create a thread-safe statistics collector for your server
3. Experiment with different thread pool sizes and analyze performance
4. Deliberately create a deadlock, then fix it

### Testing Tools
- Thread Sanitizer
- Apache Bench (ab) for concurrent testing
- wrk for more advanced load testing
- Valgrind Helgrind for thread error detection

## Stage 4: Event-driven Server

### Learning Focus
- Non-blocking I/O
- Event loop architecture
- epoll/kqueue/select mechanisms
- Callback-based programming
- Buffer management
- State machine design

### Weekly Schedule

#### Week 10: Event-driven Programming Fundamentals
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | I/O Multiplexing | Study I/O multiplexing concepts | UNIX Network Programming Ch 6, Computer Networking Ch 3.6 (Principles of congestion control) |
| 3-4 | API Details | Learn epoll/kqueue API details | Linux Programming Interface Ch 63 (Alternative I/O Models) |
| 5-7 | Architecture | Design your event loop architecture | libuv Documentation (Event Loop Overview) |

#### Week 11: Non-blocking I/O Implementation
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Non-blocking Mode | Convert socket operations to non-blocking mode | Computer Networking Ch 3.7.1 (Classic TCP congestion control), UNIX Network Programming Ch 6.2 |
| 3-4 | Event Loop | Implement basic event loop with epoll/kqueue | Linux Programming Interface Ch 63.4 (epoll API) |
| 5-7 | Event Handling | Handle read and write events properly | Systems Performance Ch 7.5 (Event Frameworks) |

#### Week 12: Event-driven HTTP Processing
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | State Machine | Design state machine for HTTP request processing | Computer Networking Ch 3.7.2 (Network-assisted congestion notification) |
| 3-5 | HTTP Parser | Implement event-driven HTTP parser | Nginx source code documentation (http_parse.c) |
| 6-7 | Connection Management | Add timeout handling and connection management | UNIX Network Programming Ch 6.5 (Timeout Processing) |

#### Week 13: Buffer Management and Optimization
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Buffer Management | Implement efficient buffer management | Computer Networking Ch 3.7.3 (Fairness in congestion control), Systems Performance Ch 8 (Memory) |
| 3-4 | Buffer Pooling | Add buffer pooling to reduce allocations | UNIX Network Programming Ch 6.8 (Buffer Sizes) |
| 5-6 | Performance | Performance testing and bottleneck identification | Computer Networking Ch 3.8 (Evolution of transport layer) |
| 7 | Optimization | Optimization and edge case handling | Systems Performance Ch 13 (TCP) |

### Implementation Milestones
- [ ] Working epoll/kqueue-based event loop
- [ ] Non-blocking socket I/O
- [ ] Event-driven HTTP request processing
- [ ] Efficient buffer management
- [ ] Connection timeout handling
- [ ] Event-based logging

### Debugging Skills for This Stage
- Debugging asynchronous code flows
- Event loop debugging
- Non-blocking I/O issues
- Using perf for performance analysis

### Practical Exercises
1. Implement a simple reactor pattern
2. Add timer events to your event loop
3. Create a buffer pool for efficient memory use
4. Benchmark against your threaded implementation

### Testing Tools
- perf for performance analysis
- strace for system call patterns
- wrk for HTTP benchmarking
- Custom scripts for long-lived connections

## Stage 5: Multi-process, Multi-core Server

### Learning Focus
- Process creation and management
- Inter-process communication
- Shared memory techniques
- Socket passing between processes
- CPU affinity
- Load balancing strategies

### Weekly Schedule

#### Week 14: Multi-process Architecture
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | Process Creation | Study process creation fundamentals | Linux Programming Interface Ch 24-25 (Process Creation) |
| 3-4 | Pre-fork Model | Implement basic pre-fork server model | UNIX Network Programming Ch 15.4 (Pre-forking Server) |
| 5-7 | Socket Passing | Add socket passing from master to workers | Linux Programming Interface Ch 61.13 (Passing File Descriptors) |

#### Week 15: Inter-process Communication
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | IPC Study | Study IPC mechanisms | Linux Programming Interface Ch 53-55 (IPC Introduction) |
| 3-4 | Shared Memory | Implement shared memory for statistics | Linux Programming Interface Ch 54 (POSIX Shared Memory) |
| 5-7 | Worker Management | Add IPC for worker management | UNIX Network Programming Ch 15.7 (IPC Between Processes) |

#### Week 16: CPU Optimization
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | CPU Study | Study CPU affinity and NUMA considerations | Computer Networking Ch 6.6.1 (Data center architectures), Systems Performance Ch 6.5 (CPU Binding) |
| 3-4 | CPU Pinning | Implement CPU pinning for workers | Linux Programming Interface Ch 35.4 (CPU Affinity) |
| 5-7 | Load Balancing | Add load balancing between workers | Computer Networking Ch 6.6.2 (Trends in data center networking), Nginx documentation (Load Balancing) |

#### Week 17: Final Optimization
| Day | Focus | Activities | Reading |
|-----|-------|------------|---------|
| 1-2 | System Testing | System-wide performance testing | Systems Performance Ch 5 (Applications) |
| 3-4 | Bottlenecks | Identify and fix bottlenecks | Systems Performance Ch 13 (TCP) |
| 5-6 | Worker Recycling | Implement graceful worker recycling | Review Nginx architecture documentation |
| 7 | Final Benchmark | Final benchmarking and documentation | Review all key concepts and architectures |

### Implementation Milestones
- [ ] Pre-fork worker process model
- [ ] Socket passing between processes
- [ ] Shared memory for statistics
- [ ] Process monitoring and management
- [ ] CPU affinity optimization
- [ ] Graceful worker process recycling
- [ ] Load balancing between workers

### Debugging Skills for This Stage
- Multi-process debugging with GDB
- Shared memory debugging
- System-wide performance analysis
- Load distribution analysis

### Practical Exercises
1. Implement a process pool with dynamic scaling
2. Create a shared memory scoreboard for monitoring
3. Experiment with different load balancing strategies
4. Compare performance across different server architectures

### Testing Tools
- Multi-process GDB
- ipcs for shared memory inspection
- htop/top for process monitoring
- perf for system-wide analysis

## Essential Debugging Skills

### Debugging Skills Progression

| Stage | Focus | Key Skills |
|-------|-------|------------|
| Stage 1 | Basic Debugging | GDB fundamentals, strace, socket states, basic logging |
| Stage 2 | Protocol Debugging | HTTP message analysis, Wireshark/tcpdump, header validation |
| Stage 3 | Concurrent Debugging | Thread Sanitizer, GDB for threads, deadlock identification |
| Stage 4 | Event Loop Debugging | Async code flow tracing, event loop state analysis, perf |
| Stage 5 | Multi-process Debugging | Multi-process GDB, shared memory inspection, system analysis |

### Stage 1: Basic Debugging
- GDB fundamentals (breakpoints, stepping, variables)
- System call tracing with strace
- Socket state examination with netstat/ss
- Basic logging implementation

### Stage 2: Protocol Debugging
- HTTP message analysis
- Wireshark/tcpdump packet capture
- HTTP header validation
- File I/O debugging

### Stage 3: Concurrent Debugging
- Thread Sanitizer for race detection
- GDB multi-threaded debugging
- Deadlock identification
- Lock contention analysis

### Stage 4: Event Loop Debugging
- Asynchronous code flow tracing
- Event loop state analysis
- Non-blocking I/O issues
- Performance analysis with perf

### Stage 5: Multi-process Debugging
- Multi-process debugging with GDB
- Shared memory inspection
- System-wide performance analysis
- Process communication debugging

## Performance Testing Progression

### Testing Progression Overview

| Stage | Focus | Tools |
|-------|-------|-------|
| Stage 1-2 | Basic Testing | curl, browsers, Apache Bench (ab) |
| Stage 3 | Concurrency Testing | Multi-client testing, thread pool optimization |
| Stage 4 | Scalability Testing | Connection limits, I/O bottlenecks |
| Stage 5 | System-wide Testing | Multi-core utilization, process distribution |

### Stage 1-2: Basic Testing
- Manual testing with curl, browsers
- Basic benchmarking with Apache Bench (ab)
- Response time measurement

### Stage 3: Concurrency Testing
- Multi-client testing
- Thread pool size optimization
- Synchronization overhead analysis

### Stage 4: Scalability Testing
- Connection handling limits
- I/O bottleneck identification
- Memory usage optimization

### Stage 5: System-wide Testing
- Multi-core utilization
- Process distribution efficiency
- Comparative benchmarking against production servers

## Knowledge Checklists

### Stage 1: Echo Server
- [ ] Socket API and TCP connection lifecycle
- [ ] Client-server communication patterns
- [ ] Robust I/O handling techniques
- [ ] Error handling for network operations
- [ ] Basic debugging methodology

### Stage 2: Sequential HTTP Server
- [ ] HTTP protocol structure and headers
- [ ] Request parsing techniques
- [ ] Response generation
- [ ] MIME type handling
- [ ] HTTP error codes and handling
- [ ] Static file serving patterns

### Stage 3: Multi-threaded Server
- [ ] Thread creation and management
- [ ] Synchronization primitives (mutexes, condition variables)
- [ ] Thread pool design patterns
- [ ] Race condition prevention
- [ ] Deadlock avoidance
- [ ] Thread-safe data structures

### Knowledge Checklists (continued)

### Stage 4: Event-driven Server
- [ ] Non-blocking I/O concepts and socket flags (O_NONBLOCK, fcntl())
- [ ] Event loops and multiplexing mechanisms (select/poll/epoll/kqueue)
- [ ] Callback patterns and event handler registration
- [ ] Buffer management for asynchronous operations
- [ ] State machine design for protocol processing
- [ ] Edge vs. level triggering in event notification

### Stage 5: Multi-process, Multi-core Server
- [ ] Process creation and management (fork(), waitpid())
- [ ] Inter-process communication techniques (pipes, shared memory)
- [ ] Shared memory usage and synchronization
- [ ] File descriptor passing between processes
- [ ] CPU affinity concepts and NUMA considerations
- [ ] Load balancing strategies for connection distribution
- [ ] Multi-core optimization and resource allocation

## Career Impact by Stage

| Stage | Impressiveness | Competitive For | Key Skills Demonstrated |
|-------|---------------|-----------------|------------------------|
| 1-2 | 6/10 | Junior backend roles | Network programming, protocol implementation |
| 3 | 7.5/10 | Most entry-level SWE roles | Concurrency, synchronization, resource management |
| 4 | 8.5/10 | Systems engineering | Async programming, event-driven design, scalable I/O |
| 5 | 9/10 | Performance engineering | Multi-core optimization, system architecture |

### Stage 1-2: Echo Server & Sequential HTTP Server
- **Competitive for**: Junior backend roles, entry-level web developer positions
- **Skills demonstrated**: Network programming, protocol implementation, I/O handling

### Stage 3: Multi-threaded Server
- **Competitive for**: Backend engineering, most entry-level SWE roles at tech companies
- **Skills demonstrated**: Concurrency, synchronization, resource management

### Stage 4: Event-driven Server
- **Competitive for**: Systems engineering, infrastructure teams, high-performance service development
- **Skills demonstrated**: Asynchronous programming, event-driven design, scalable I/O

### Stage 5: Multi-process, Multi-core Server
- **Competitive for**: Performance engineering, distributed systems teams, platform engineering
- **Skills demonstrated**: Multi-core optimization, process management, system architecture

## Adjustable Timeline

This schedule can be adjusted based on your available time:

| Pace | Timeframe | Hours/Week | Recommended For |
|------|-----------|------------|-----------------|
| Accelerated | 8-10 weeks | 30-40 hours | Full-time focus, prior C experience |
| Standard | 15-17 weeks | 15-20 hours | Part-time project with other commitments |
| Extended | 25-30 weeks | 8-10 hours | Working professionals with limited time |

For an extended timeline, consider:
- Completing Stage 1 & 2 in double the recommended time
- Creating project milestones with more granular weekly goals
- Focusing on thorough understanding before moving to the next stage
- Setting aside dedicated reading days separate from implementation days

## Measuring Success

Track your progress with these metrics:

### Performance Metrics
- Requests per second
- Maximum concurrent connections
- Memory usage per connection
- Response time under load

### Code Quality Metrics
- Lines of code (lower is often better!)
- Test coverage
- Known bugs and issues
- Memory leaks

### Knowledge Acquisition
- Complete the knowledge checklist for each stage
- Successfully implement the practical exercises
- Master the debugging skills for each stage

## Decision Point After Stage 3

After completing Stage 3 (Multi-threaded Server), you'll have:
- A functional, concurrent HTTP server
- Experience with threading and synchronization
- A solid project for your portfolio
- Competitive skills for many entry-level positions

At this point, evaluate:
- Your enjoyment of the project
- Your career goals and interests
- The value of additional stages for your specific path

Whether you continue to Stages 4-5 or pivot to a new project, the skills gained through Stage 3 will already make you a strong candidate for entry-level software engineering positions.
