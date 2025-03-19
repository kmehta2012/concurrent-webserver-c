# Master Plan: Concurrent Web Server Development Project

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
   - Primary reference for fundamentals
   - Key chapters: 10 (I/O), 11 (Networking), 12 (Concurrency)

2. **The Art of Debugging with GDB, DDD, and Eclipse**
   - Essential for debugging techniques
   - Key chapters: 1-4 (Basics), 5 (Multi-threaded debugging)

3. **The Linux Programming Interface**
   - Advanced Linux programming concepts
   - Key chapters: 24-27 (Processes), 53-55 (IPC), 59-63 (Alternative I/O)

4. **UNIX Network Programming, Volume 1**
   - Classic text on network programming
   - Key chapters: 6 (I/O Multiplexing), 15 (IPC)

5. **Systems Performance: Enterprise and the Cloud**
   - Performance analysis and optimization
   - Key chapters: A5-6 (Applications, CPUs), 7-8 (Memory, Files), 13 (TCP)

6. **HTTP: The Definitive Guide**
   - HTTP protocol details
   - Key chapters: 1-3 (HTTP fundamentals)

### Online Resources
1. **POSIX Threads Programming Tutorial**
   - https://computing.llnl.gov/tutorials/pthreads/
   - Practical guide to threads programming

2. **Operating Systems: Three Easy Pieces**
   - http://pages.cs.wisc.edu/~remzi/OSTEP/
   - Excellent concurrency chapters

3. **libuv Documentation**
   - http://docs.libuv.org/
   - Guide to event loop concepts

4. **Nginx Architecture Documentation**
   - https://nginx.org/en/docs/
   - Real-world server architecture

5. **RFC 7230: HTTP/1.1 Message Syntax and Routing**
   - Official HTTP specification

### Debugging Tools Documentation
1. **GDB Documentation** - https://sourceware.org/gdb/current/onlinedocs/gdb/
2. **Valgrind User Manual** - https://valgrind.org/docs/
3. **Thread Sanitizer Documentation** - https://github.com/google/sanitizers/wiki
4. **Performance Analysis Tools**: perf, strace, tcpdump, Wireshark

### Resource Priority by Stage

| Stage | Primary Resources | Supporting Resources |
|-------|------------------|---------------------|
| Stage 1 | CSAPP (Ch 10-11), Art of Debugging (Ch 1-4) | GDB Documentation, strace manual |
| Stage 2 | CSAPP (Ch 11.5-11.6), HTTP: Definitive Guide (Ch 1-3) | RFC 7230, Wireshark Documentation |
| Stage 3 | CSAPP (Ch 12), POSIX Threads Tutorial | Art of Debugging (Ch 5), OSTEP Concurrency Chapters |
| Stage 4 | Linux Programming Interface (Ch 59-63), UNIX Network Programming (Ch 6) | libuv Documentation, Systems Performance (Ch 6-8) |
| Stage 5 | Linux Programming Interface (Ch 24-27, 53-55), UNIX Network Programming (Ch 15) | Systems Performance (Ch 5,6,13), Nginx Documentation |

## Project Stages Overview

| Stage | Focus | Duration | Impressiveness |
|-------|-------|----------|---------------|
| 1: Echo Server | Network foundations, Socket programming | 2 weeks | 4/10 |
| 2: Sequential HTTP Server | HTTP protocol, Static file serving | 3 weeks | 6/10 |
| 3: Multi-threaded Server | Thread programming, Synchronization | 4 weeks | 7.5/10 |
| 4: Event-driven Server | Non-blocking I/O, Event loops | 4 weeks | 8.5/10 |
| 5: Multi-process Server | Process management, IPC, Multi-core | 4 weeks | 9/10 |

## Stage 1: Echo Server

### Learning Focus
- Socket programming fundamentals
- Client-server architecture
- Basic network I/O
- Error handling and robustness
- Debugging networked applications

### Weekly Schedule

#### Week 1: Network Programming Foundations
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Socket API | Study socket API and TCP/IP basics from CSAPP Ch. 11 |
| 3-4 | Client Implementation | Implement basic client-socket functions |
| 5-7 | Server Implementation | Implement basic server-socket functions and logging |

#### Week 2: Robustness and Debugging
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Robust I/O | Study robust I/O patterns from CSAPP Ch. 10 |
| 3-4 | Error Handling | Implement robust read/write functions with error handling |
| 5-6 | Debugging | Learn GDB basics and debug your implementation |
| 7 | Finalization | Add comprehensive error handling and logging |

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
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | HTTP Basics | Study HTTP protocol basics from HTTP: The Definitive Guide |
| 3-4 | Design | Design HTTP request parser and response generator |
| 5-7 | Request Parsing | Implement basic HTTP request parsing |

#### Week 4: Request Handling and Static Files
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Response Generation | Implement HTTP response generation |
| 3-4 | File Serving | Add static file serving capabilities |
| 5-7 | MIME Types | Implement proper MIME type detection and handling |

#### Week 5: Error Handling and Logging
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Error Responses | Implement proper HTTP error responses (404, 500, etc.) |
| 3-4 | Logging | Add comprehensive HTTP access and error logging |
| 5-6 | Testing | Test with various browsers and HTTP clients |
| 7 | Performance | Performance testing and basic optimizations |

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
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Thread Basics | Study threading basics from CSAPP Ch. 12 |
| 3-4 | Thread Creation | Implement basic thread creation and management |
| 5-7 | Synchronization | Practice with mutexes and condition variables |

#### Week 7: Thread Safety
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Safety Analysis | Identify thread safety issues in your server |
| 3-4 | Safe Logging | Implement thread-safe logging |
| 5-7 | Safe Handling | Make request handling thread-safe |

#### Week 8: Thread Pool Implementation
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Pool Design | Design thread pool architecture |
| 3-5 | Worker Threads | Implement worker thread pool |
| 6-7 | Job Queue | Add job queue and work distribution |

#### Week 9: Testing and Optimization
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Race Detection | Use Thread Sanitizer to identify race conditions |
| 3-4 | Bug Fixing | Fix synchronization issues |
| 5-6 | Load Testing | Load test with concurrent connections |
| 7 | Optimization | Performance tuning and optimization |

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
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | I/O Multiplexing | Study I/O multiplexing from UNIX Network Programming |
| 3-4 | API Details | Learn epoll/kqueue API details |
| 5-7 | Architecture | Design your event loop architecture |

#### Week 11: Non-blocking I/O Implementation
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Non-blocking Mode | Convert socket operations to non-blocking mode |
| 3-4 | Event Loop | Implement basic event loop with epoll/kqueue |
| 5-7 | Event Handling | Handle read and write events properly |

#### Week 12: Event-driven HTTP Processing
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | State Machine | Design state machine for HTTP request processing |
| 3-5 | HTTP Parser | Implement event-driven HTTP parser |
| 6-7 | Connection Management | Add timeout handling and connection management |

#### Week 13: Buffer Management and Optimization
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Buffer Management | Implement efficient buffer management |
| 3-4 | Buffer Pooling | Add buffer pooling to reduce allocations |
| 5-6 | Performance | Performance testing and bottleneck identification |
| 7 | Optimization | Optimization and edge case handling |

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
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | Process Creation | Study process creation from Linux Programming Interface |
| 3-4 | Pre-fork Model | Implement basic pre-fork server model |
| 5-7 | Socket Passing | Add socket passing from master to workers |

#### Week 15: Inter-process Communication
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | IPC Study | Study IPC mechanisms |
| 3-4 | Shared Memory | Implement shared memory for statistics |
| 5-7 | Worker Management | Add IPC for worker management |

#### Week 16: CPU Optimization
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | CPU Study | Study CPU affinity and NUMA considerations |
| 3-4 | CPU Pinning | Implement CPU pinning for workers |
| 5-7 | Load Balancing | Add load balancing between workers |

#### Week 17: Final Optimization
| Day | Focus | Activities |
|-----|-------|------------|
| 1-2 | System Testing | System-wide performance testing |
| 3-4 | Bottlenecks | Identify and fix bottlenecks |
| 5-6 | Worker Recycling | Implement graceful worker recycling |
| 7 | Final Benchmark | Final benchmarking and documentation |

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

### Stage 4: Event-driven Server
- [ ] Non-blocking I/O concepts
- [ ] Event loops and multiplexing
- [ ] Callback patterns
- [ ] Buffer management
- [ ] State machine design
- [ ] Edge vs. level triggering

### Stage 5: Multi-process, Multi-core Server
- [ ] Process creation and management
- [ ] Inter-process communication techniques
- [ ] Shared memory usage
- [ ] File descriptor passing
- [ ] CPU affinity concepts
- [ ] Load balancing strategies
- [ ] Multi-core optimization

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

This 17-week schedule can be adjusted based on your available time:

| Pace | Timeframe | Hours/Week |
|------|-----------|------------|
| Accelerated | 8-10 weeks | 30-40 hours |
| Standard | 17 weeks | 15-20 hours |
| Extended | 25-30 weeks | 8-10 hours |

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
