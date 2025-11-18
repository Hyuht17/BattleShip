import { useState, useRef, useEffect } from 'react';
import './ChatBox.css';

function ChatBox({ messages, onSendMessage, currentUser }) {
  const [inputMessage, setInputMessage] = useState('');
  const messagesEndRef = useRef(null);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  const handleSubmit = (e) => {
    e.preventDefault();
    if (inputMessage.trim()) {
      onSendMessage(inputMessage);
      setInputMessage('');
    }
  };

  return (
    <div className="chat-box">
      <h4>💬 Chat</h4>
      <div className="chat-messages">
        {messages.length === 0 ? (
          <div className="no-messages">No messages yet</div>
        ) : (
          messages.map((msg, index) => (
            <div
              key={index}
              className={`chat-message ${msg.from === currentUser ? 'own' : 'other'}`}
            >
              <div className="message-header">
                <span className="message-author">{msg.from}</span>
                <span className="message-time">{msg.timestamp}</span>
              </div>
              <div className="message-content">{msg.message}</div>
            </div>
          ))
        )}
        <div ref={messagesEndRef} />
      </div>

      <form onSubmit={handleSubmit} className="chat-input-form">
        <input
          type="text"
          value={inputMessage}
          onChange={(e) => setInputMessage(e.target.value)}
          placeholder="Type a message..."
          className="chat-input"
        />
        <button type="submit" className="chat-send-button">
          📤
        </button>
      </form>
    </div>
  );
}

export default ChatBox;
