# Testing Checklist - BattleShip Game

## ✅ Pre-Testing Setup

- [ ] Node.js version >= 14 installed
- [ ] All dependencies installed (`npm install`)
- [ ] C++ server compiled successfully
- [ ] All 3 servers can start without errors

## 🧪 Unit Tests

### C++ Server Tests
- [ ] Server starts on port 8080
- [ ] Accepts TCP connections
- [ ] Creates users.dat file on first registration
- [ ] Multiple clients can connect simultaneously

### Node.js Middleware Tests
- [ ] Server starts on port 3000
- [ ] WebSocket connections work
- [ ] Can connect to C++ server on port 8080
- [ ] Messages are properly forwarded between frontend and C++ server

### Frontend Tests
- [ ] Vite dev server starts on port 5173
- [ ] All components render without errors
- [ ] Socket.IO connection established
- [ ] No console errors

## 🎮 Functional Tests

### 1. Authentication
- [ ] Register new account with valid credentials
- [ ] Register fails with existing username
- [ ] Login with correct credentials succeeds
- [ ] Login with wrong credentials fails
- [ ] User stays logged in until disconnect

### 2. Lobby System
- [ ] Player list displays after login
- [ ] Refresh button updates player list
- [ ] Current user not shown in player list
- [ ] Online status displayed correctly
- [ ] Players in game marked as unavailable

### 3. Challenge System
- [ ] Can challenge available players
- [ ] Challenge request received by target player
- [ ] Accept challenge starts game
- [ ] Reject challenge notifies challenger
- [ ] Cannot challenge players already in game

### 4. Ship Placement
- [ ] All 5 ships must be placed
- [ ] Can toggle horizontal/vertical orientation
- [ ] Ships cannot overlap
- [ ] Ships cannot go out of bounds
- [ ] Reset button clears all ships
- [ ] Confirm disabled until all ships placed
- [ ] Game starts when both players ready

### 5. Gameplay
- [ ] Turn indicator shows correctly
- [ ] Can only attack on your turn
- [ ] Cannot attack same cell twice
- [ ] Hit shows 💥 on opponent's board
- [ ] Miss shows 💧 on opponent's board
- [ ] Turn switches after move
- [ ] Ship sunk notification appears
- [ ] Win condition detected correctly
- [ ] Lose condition detected correctly

### 6. Chat System
- [ ] Can send messages during game
- [ ] Messages appear in real-time
- [ ] Own messages aligned right
- [ ] Opponent messages aligned left
- [ ] Timestamps displayed correctly
- [ ] Chat scrolls to bottom on new message

### 7. Disconnect Handling
- [ ] Player disconnect during lobby updates player list
- [ ] Player disconnect during game notifies opponent
- [ ] Opponent wins if player disconnects
- [ ] Game session cleaned up after disconnect

## 🔥 Stress Tests

- [ ] 10 concurrent users can register/login
- [ ] 5 simultaneous games running
- [ ] Server handles rapid message sending
- [ ] No memory leaks after multiple games
- [ ] Reconnection works after brief disconnect

## 📱 UI/UX Tests

- [ ] Responsive design on mobile (< 768px)
- [ ] Responsive design on tablet (768px - 1024px)
- [ ] All buttons have hover effects
- [ ] Loading states shown appropriately
- [ ] Error messages are clear and helpful
- [ ] Success messages displayed
- [ ] Animations smooth and not distracting

## 🐛 Edge Cases

- [ ] Empty username/password rejected
- [ ] Very long usernames handled
- [ ] Special characters in username/password
- [ ] Rapid clicking doesn't cause issues
- [ ] Browser back button doesn't break state
- [ ] Multiple tabs same user (should handle gracefully)
- [ ] Network interruption recovery

## 🔒 Security Tests (Basic)

- [ ] Passwords not stored in plain text visible in logs
- [ ] Cannot see other players' ship positions
- [ ] Cannot make moves out of turn
- [ ] Cannot place ships after game started
- [ ] Input validation prevents injection

## 📊 Performance Tests

- [ ] Page loads in < 3 seconds
- [ ] Ship placement responsive (< 100ms)
- [ ] Move response time < 500ms
- [ ] Chat message latency < 200ms
- [ ] Player list updates within 1 second

## 🎯 Integration Tests

### Full Game Flow (2 Players)
1. [ ] Player 1: Register → Login → Lobby
2. [ ] Player 2: Register → Login → Lobby
3. [ ] Player 1: Challenge Player 2
4. [ ] Player 2: Accept challenge
5. [ ] Both: Place ships
6. [ ] Both: Play complete game
7. [ ] Winner declared correctly
8. [ ] Both: Return to lobby
9. [ ] Can start new game

### Concurrent Games
1. [ ] Game 1: Player A vs Player B
2. [ ] Game 2: Player C vs Player D  
3. [ ] Both games run independently
4. [ ] No message cross-contamination
5. [ ] Both games can finish successfully

## 📝 Test Results Summary

**Date Tested:** _______________

**Tester:** _______________

**Environment:**
- OS: _______________
- Node.js Version: _______________
- Browser: _______________

**Overall Status:** ⬜ Pass | ⬜ Fail | ⬜ Partial

**Critical Issues Found:**
1. 
2. 
3. 

**Minor Issues Found:**
1. 
2. 
3. 

**Recommendations:**
1. 
2. 
3. 

---

## 🚀 Quick Test Commands

```bash
# Start all servers
./start-all.sh

# Test C++ server connection
nc -zv localhost 8080

# Test Node.js server
curl http://localhost:3000/health

# Test frontend
curl http://localhost:5173

# View C++ server logs
tail -f server/server.log

# View Node.js logs  
cd node-server && npm start 2>&1 | tee server.log

# Monitor network traffic
tcpdump -i lo port 8080
```

---

## ✨ Test Coverage Goals

- [ ] 80%+ feature coverage
- [ ] All critical paths tested
- [ ] Major edge cases handled
- [ ] Performance benchmarks met
- [ ] No critical bugs remaining

**Testing Completed:** ⬜ Yes | ⬜ No | ⬜ In Progress
