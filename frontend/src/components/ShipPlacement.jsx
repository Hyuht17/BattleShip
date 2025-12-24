import { useState } from 'react';
import './ShipPlacement.css';

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
    let className = 'cell';

    const hoverCell = hoverCells.find(c => c.row === row && c.col === col);
    if (hoverCell) {
      className += hoverCell.valid ? ' hover-valid' : ' hover-invalid';
    }

    if (board[row][col] > 0) {
      className += ' ship';
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
    <div className="ship-placement">
      <h3>📍 Place Your Ships</h3>

      <div className="placement-content">
        <div className="placement-board">
          <div className="board-grid">
            <div className="cell header"></div>
            {Array.from({ length: 10 }, (_, i) => (
              <div key={`col-${i}`} className="cell header">{i}</div>
            ))}

            {'ABCDEFGHIJ'.split('').map((row, rowIndex) => (
              <>
                <div key={`row-${row}`} className="cell header">{row}</div>
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
              </>
            ))}
          </div>
        </div>

        <div className="placement-controls">
          <div className="ships-list">
            <h4>Ships to Place:</h4>
            {SHIPS.map((ship, index) => (
              <div
                key={ship.name}
                className={`ship-item ${index === currentShip ? 'current' : ''} ${index < currentShip ? 'placed' : ''}`}
              >
                <span className="ship-symbol">{ship.symbol}</span>
                <span className="ship-name">{ship.name}</span>
                <span className="ship-size">({ship.size} cells)</span>
                {index < currentShip && <span className="checkmark">✓</span>}
              </div>
            ))}
          </div>

          <div className="orientation-toggle">
            <label>Orientation:</label>
            <div className="toggle-buttons">
              <button
                className={orientation === 'horizontal' ? 'active' : ''}
                onClick={() => setOrientation('horizontal')}
              >
                ↔️ Horizontal
              </button>
              <button
                className={orientation === 'vertical' ? 'active' : ''}
                onClick={() => setOrientation('vertical')}
              >
                ↕️ Vertical
              </button>
            </div>
          </div>

          <div className="action-buttons">
            <button onClick={handleReset} className="reset-button">
              🔄 Reset
            </button>
            <button
              onClick={handleConfirm}
              className="confirm-button"
              disabled={placedShips.length !== SHIPS.length}
            >
              ✓ Confirm Placement
            </button>
          </div>

          <div className="placement-hint">
            <p>💡 <strong>Left-click</strong> to place the current ship</p>
            <p>💡 <strong>Right-click</strong> to rotate orientation</p>
            <p>💡 Or use the buttons to toggle orientation</p>
          </div>
        </div>
      </div>

      {/* Local Notification Modal */}
      {localNotification && (
        <div className="fixed inset-0 bg-black bg-opacity-30 flex items-center justify-center z-50">
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
