import { useState, useRef, useEffect } from 'react';

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
    <div className="flex flex-col h-full">
      <h4 className="text-lg font-bold text-gray-800 mb-4 pb-3 border-b-2 border-gray-200">💬 Chat</h4>
      
      <div className="flex-1 overflow-y-auto p-3 bg-gray-50 rounded-lg mb-4 space-y-3 max-h-[200px]">
        {messages.length === 0 ? (
          <div className="text-center py-12 text-gray-400 italic">
            <div className="text-4xl mb-2 opacity-50">💬</div>
            <p>No messages yet</p>
          </div>
        ) : (
          messages.map((msg, index) => (
            <div
              key={index}
              className={`p-3 rounded-lg max-w-[85%] break-words shadow-sm ${
                msg.from === currentUser
                  ? 'bg-blue-500 text-white ml-auto rounded-br-sm'
                  : 'bg-white text-gray-800 mr-auto rounded-bl-sm border border-gray-200'
              }`}
            >
              <div className={`flex justify-between items-center gap-3 mb-2 text-xs pb-2 border-b ${
                msg.from === currentUser ? 'border-white/20' : 'border-gray-200'
              }`}>
                <span className="font-bold uppercase tracking-wide">{msg.from}</span>
                <span className={`${msg.from === currentUser ? 'text-white/70' : 'text-gray-500'}`}>
                  {msg.timestamp}
                </span>
              </div>
              <div className="text-sm leading-relaxed">{msg.message}</div>
            </div>
          ))
        )}
        <div ref={messagesEndRef} />
      </div>

      <form onSubmit={handleSubmit} className="flex gap-2 pt-4 border-t-2 border-gray-200">
        <input
          type="text"
          value={inputMessage}
          onChange={(e) => setInputMessage(e.target.value)}
          placeholder="Type a message..."
          className="flex-1 px-4 py-2 border-2 border-gray-200 rounded-lg text-sm focus:outline-none focus:border-blue-500 focus:ring-2 focus:ring-blue-200 transition-all bg-white"
        />
        <button 
          type="submit"
          className="px-4 py-2 bg-blue-500 text-white rounded-lg font-semibold hover:bg-blue-600 transition-colors text-lg shadow-md hover:shadow-lg"
        >
          📤
        </button>
      </form>
    </div>
  );
}

export default ChatBox;
