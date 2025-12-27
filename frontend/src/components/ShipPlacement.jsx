import { useState } from 'react';
import React from 'react';

const SHIPS = [
  { name: 'Carrier', size: 5, symbol: '🛳️' },
  { name: 'Battleship', size: 4, symbol: '🚢' },
  { name: 'Cruiser', size: 3, symbol: '⛴️' },
  { name: 'Submarine', size: 3, symbol: '🚤' },
  { name: 'Destroyer', size: 2, symbol: '⛵' }
];

function ShipPlacement({ onPlaceShips }) {
  const [board, setBoard] = useState(Array(10).fill(null).map(() => Array(10).fill(0)));
  const [placedShips, setPlacedShips] = useState([]);
  const [currentShip, setCurrentShip] = useState(0);
  const [orientation, setOrientation] = useState('horizontal');
  const [hoverCells, setHoverCells] = useState([]);
  const [localNotification, setLocalNotification] = useState(null);

  const canPlaceShip = (row, col, size, horizontal) => {
    if (horizontal) {
      if (col + size > 10) return false;
      for (let i = 0; i < size; i++) {
        if (board[row][col + i] !== 0) return false;
      }
    } else {
      if (row + size > 10) return false;
      for (let i = 0; i < size; i++) {
        if (board[row + i][col] !== 0) return false;
      }
    }
    return true;
  };

  const handleCellHover = (row, col) => {
    if (currentShip >= SHIPS.length) return;

    const ship = SHIPS[currentShip];
    const horizontal = orientation === 'horizontal';
    const cells = [];

    if (canPlaceShip(row, col, ship.size, horizontal)) {
      for (let i = 0; i < ship.size; i++) {
        cells.push({
          row: horizontal ? row : row + i,
          col: horizontal ? col + i : col,
          valid: true
        });
      }
    } else {
      for (let i = 0; i < ship.size; i++) {
        const r = horizontal ? row : row + i;
        const c = horizontal ? col + i : col;
        if (r < 10 && c < 10) {
          cells.push({ row: r, col: c, valid: false });
        }
      }
    }

    setHoverCells(cells);
  };

  const handleCellClick = (row, col) => {
    if (currentShip >= SHIPS.length) return;

    const ship = SHIPS[currentShip];
    const horizontal = orientation === 'horizontal';

    if (!canPlaceShip(row, col, ship.size, horizontal)) {
      setLocalNotification({
        title: 'Không thể đặt tàu',
        message: 'Không thể đặt tàu ở vị trí này! Vui lòng chọn vị trí khác.',
        type: 'error'
      });
      return;
    }

    // Place ship on board
    const newBoard = board.map(r => [...r]);
    for (let i = 0; i < ship.size; i++) {
      const r = horizontal ? row : row + i;
      const c = horizontal ? col + i : col;
      newBoard[r][c] = currentShip + 1;
    }

    setBoard(newBoard);
    setPlacedShips([...placedShips, {
      name: ship.name,
      size: ship.size,
      symbol: ship.symbol,
      row,
      col,
      horizontal
    }]);
    setCurrentShip(currentShip + 1);
    setHoverCells([]);
  };

  const handleRightClick = (e, row, col) => {
    e.preventDefault(); // Ngăn context menu mặc định
    
    // Toggle orientation
    setOrientation(prev => prev === 'horizontal' ? 'vertical' : 'horizontal');
    
    // Update hover cells với orientation mới
    handleCellHover(row, col);
  };

  const handleReset = () => {
    setBoard(Array(10).fill(null).map(() => Array(10).fill(0)));
    setPlacedShips([]);
    setCurrentShip(0);
    setHoverCells([]);
  };

  const handleRandomPlace = () => {
    // Reset board first
    const newBoard = Array(10).fill(null).map(() => Array(10).fill(0));
    const newPlacedShips = [];
    
    // Try to place all ships randomly
    const shuffledShips = [...SHIPS].sort(() => Math.random() - 0.5);
    let attempts = 0;
    const maxAttempts = 1000;

    for (const ship of shuffledShips) {
      let placed = false;
      let attemptCount = 0;

      while (!placed && attemptCount < maxAttempts) {
        attemptCount++;
        attempts++;

        // Random position and orientation
        const row = Math.floor(Math.random() * 10);
        const col = Math.floor(Math.random() * 10);
        const horizontal = Math.random() > 0.5;

        // Check if can place
        if (canPlaceShipOnBoard(newBoard, row, col, ship.size, horizontal)) {
          // Place ship
          for (let i = 0; i < ship.size; i++) {
            const r = horizontal ? row : row + i;
            const c = horizontal ? col + i : col;
            newBoard[r][c] = 1; // Mark as occupied
          }

          newPlacedShips.push({
            name: ship.name,
            size: ship.size,
            symbol: ship.symbol,
            row,
            col,
            horizontal
          });

          placed = true;
        }
      }

      if (!placed) {
        // If we can't place all ships, reset and try again
        setLocalNotification({
          title: 'Không thể đặt ngẫu nhiên',
          message: 'Không thể tìm vị trí phù hợp. Vui lòng thử lại!',
          type: 'warning'
        });
        return;
      }
    }

    // Update board with ship indices for display
    const displayBoard = Array(10).fill(null).map(() => Array(10).fill(0));
    newPlacedShips.forEach((ship, shipIndex) => {
      for (let i = 0; i < ship.size; i++) {
        const r = ship.horizontal ? ship.row : ship.row + i;
        const c = ship.horizontal ? ship.col + i : ship.col;
        displayBoard[r][c] = shipIndex + 1;
      }
    });

    setBoard(displayBoard);
    setPlacedShips(newPlacedShips);
    setCurrentShip(SHIPS.length);
    setHoverCells([]);
  };

  const canPlaceShipOnBoard = (board, row, col, size, horizontal) => {
    if (horizontal) {
      if (col + size > 10) return false;
      for (let i = 0; i < size; i++) {
        if (board[row][col + i] !== 0) return false;
      }
    } else {
      if (row + size > 10) return false;
      for (let i = 0; i < size; i++) {
        if (board[row + i][col] !== 0) return false;
      }
    }
    return true;
  };

  const handleConfirm = () => {
    if (placedShips.length !== SHIPS.length) {
      setLocalNotification({
        title: 'Chưa đặt đủ tàu',
        message: 'Vui lòng đặt tất cả các tàu trước khi xác nhận!',
        type: 'warning'
      });
      return;
    }
    onPlaceShips(placedShips);
  };

  const getCellClass = (row, col) => {
    let className = "aspect-square bg-white text-lg font-bold flex items-center justify-center transition-all relative cursor-crosshair";
    
    const hoverCell = hoverCells.find(c => c.row === row && c.col === col);
    if (hoverCell) {
      if (hoverCell.valid) {
        className += " bg-green-100 border-2 border-green-400 cursor-pointer";
      } else {
        className += " bg-red-100 border-2 border-red-400 cursor-not-allowed";
      }
    }

    if (board[row][col] > 0) {
      className += " bg-green-200 cursor-default";
    }

    return className;
  };

  const getShipSymbol = (row, col) => {
    const cellValue = board[row][col];
    if (cellValue === 0) return null;
    // cellValue là currentShip + 1, nên shipIndex = cellValue - 1
    const shipIndex = cellValue - 1;
    return SHIPS[shipIndex]?.symbol || '🚢';
  };

  return (
    <div className="w-full max-w-7xl mx-auto bg-white p-6">
      <h3 className="text-2xl font-bold text-gray-800 mb-6">📍 Place Your Ships</h3>

      <div className="flex flex-col lg:flex-row gap-6">
        {/* Board */}
        <div className="flex-1">
          <div className="bg-white border border-gray-200 rounded-lg p-4 shadow-md">
            <div className="grid grid-cols-11 gap-0.5 bg-gray-300 p-0.5 rounded-lg">
              <div className="aspect-square bg-gray-600 text-white text-xs font-bold flex items-center justify-center rounded"></div>
              {Array.from({ length: 10 }, (_, i) => (
                <div key={`col-${i}`} className="aspect-square bg-gray-600 text-white text-xs font-bold flex items-center justify-center rounded">
                  {i}
                </div>
              ))}

              {'ABCDEFGHIJ'.split('').map((row, rowIndex) => (
                <React.Fragment key={`row-${row}`}>
                  <div className="aspect-square bg-gray-600 text-white text-xs font-bold flex items-center justify-center rounded">
                    {row}
                  </div>
                  {Array.from({ length: 10 }, (_, colIndex) => (
                    <div
                      key={`${row}-${colIndex}`}
                      className={getCellClass(rowIndex, colIndex)}
                      onClick={() => handleCellClick(rowIndex, colIndex)}
                      onContextMenu={(e) => handleRightClick(e, rowIndex, colIndex)}
                      onMouseEnter={() => handleCellHover(rowIndex, colIndex)}
                      onMouseLeave={() => setHoverCells([])}
                    >
                      {board[rowIndex][colIndex] > 0 && getShipSymbol(rowIndex, colIndex)}
                    </div>
                  ))}
                </React.Fragment>
              ))}
            </div>
          </div>
        </div>

        {/* Controls */}
        <div className="w-full lg:w-80 flex flex-col gap-4">
          {/* Ships List */}
          <div className="bg-white border border-gray-200 rounded-lg p-4 shadow-md">
            <h4 className="text-sm font-semibold text-gray-600 uppercase mb-3">Ships to Place:</h4>
            <div className="space-y-2">
              {SHIPS.map((ship, index) => (
                <div
                  key={ship.name}
                  className={`p-3 rounded-lg border-2 transition-all ${
                    index === currentShip 
                      ? 'border-blue-500 bg-blue-50' 
                      : index < currentShip 
                        ? 'border-green-500 bg-green-50' 
                        : 'border-gray-200 bg-gray-50'
                  }`}
                >
                  <div className="flex items-center justify-between">
                    <div className="flex items-center gap-2">
                      <span className="text-xl">{ship.symbol}</span>
                      <div className="flex flex-col">
                        <span className="text-sm font-semibold text-gray-800">{ship.name}</span>
                        <span className="text-xs text-gray-500">({ship.size} cells)</span>
                      </div>
                    </div>
                    {index < currentShip && (
                      <span className="text-green-600 font-bold">✓</span>
                    )}
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* Orientation Toggle */}
          <div className="bg-white border border-gray-200 rounded-lg p-4 shadow-md">
            <label className="block text-sm font-semibold text-gray-600 uppercase mb-3">Orientation:</label>
            <div className="flex gap-2">
              <button
                className={`flex-1 px-4 py-2 rounded-lg font-semibold transition-colors ${
                  orientation === 'horizontal'
                    ? 'bg-blue-500 text-white'
                    : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
                }`}
                onClick={() => setOrientation('horizontal')}
              >
                ↔️ Horizontal
              </button>
              <button
                className={`flex-1 px-4 py-2 rounded-lg font-semibold transition-colors ${
                  orientation === 'vertical'
                    ? 'bg-blue-500 text-white'
                    : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
                }`}
                onClick={() => setOrientation('vertical')}
              >
                ↕️ Vertical
              </button>
            </div>
          </div>

          {/* Action Buttons */}
          <div className="bg-white border border-gray-200 rounded-lg p-4 shadow-md">
            <div className="flex flex-col gap-2">
              <button 
                onClick={handleRandomPlace}
                className="w-full px-4 py-2 bg-purple-500 text-white rounded-lg font-semibold hover:bg-purple-600 transition-colors flex items-center justify-center gap-2"
              >
                <span>🎲</span>
                <span>Random Place</span>
              </button>
              <button 
                onClick={handleReset}
                className="w-full px-4 py-2 bg-gray-100 text-gray-700 border border-gray-200 rounded-lg font-semibold hover:bg-gray-200 transition-colors"
              >
                🔄 Reset
              </button>
              <button
                onClick={handleConfirm}
                disabled={placedShips.length !== SHIPS.length}
                className="w-full px-4 py-2 bg-blue-500 text-white rounded-lg font-semibold hover:bg-blue-600 transition-colors disabled:bg-gray-300 disabled:text-gray-500 disabled:cursor-not-allowed"
              >
                ✓ Confirm Placement
              </button>
            </div>
          </div>

          {/* Hints */}
          <div className="bg-blue-50 border border-blue-200 rounded-lg p-4">
            <h4 className="text-sm font-semibold text-blue-800 mb-2">💡 Tips:</h4>
            <ul className="text-xs text-blue-700 space-y-1">
              <li>• <strong>Left-click</strong> to place the current ship</li>
              <li>• <strong>Right-click</strong> to rotate orientation</li>
              <li>• Or use the buttons to toggle orientation</li>
            </ul>
          </div>
        </div>
      </div>

      {/* Local Notification Modal */}
      {localNotification && (
        <div className="fixed inset-0 bg-black/40 backdrop-blur-md flex items-center justify-center z-50">
          <div className="bg-white rounded-lg shadow-xl p-6 max-w-md w-full mx-4 animate-fadeIn">
            <div className="text-center">
              <div className={`w-16 h-16 mx-auto mb-4 rounded-full flex items-center justify-center ${
                localNotification.type === 'error' ? 'bg-red-100' :
                localNotification.type === 'success' ? 'bg-green-100' :
                localNotification.type === 'warning' ? 'bg-yellow-100' :
                'bg-blue-100'
              }`}>
                {localNotification.type === 'error' && (
                  <svg className="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M6 18L18 6M6 6l12 12" />
                  </svg>
                )}
                {localNotification.type === 'success' && (
                  <svg className="w-8 h-8 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M5 13l4 4L19 7" />
                  </svg>
                )}
                {localNotification.type === 'warning' && (
                  <svg className="w-8 h-8 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
                  </svg>
                )}
                {localNotification.type === 'info' && (
                  <svg className="w-8 h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                )}
              </div>
              <h3 className="text-xl font-bold text-gray-800 mb-2">{localNotification.title}</h3>
              <p className="text-gray-600 mb-6">{localNotification.message}</p>
              <button
                onClick={() => setLocalNotification(null)}
                className="px-6 py-2 bg-gray-800 hover:bg-gray-700 text-white rounded-lg font-semibold transition-colors"
              >
                Đóng
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

export default ShipPlacement;
