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
      alert('Cannot place ship here!');
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
      row,
      col,
      horizontal
    }]);
    setCurrentShip(currentShip + 1);
    setHoverCells([]);
  };

  const handleReset = () => {
    setBoard(Array(10).fill(null).map(() => Array(10).fill(0)));
    setPlacedShips([]);
    setCurrentShip(0);
    setHoverCells([]);
  };

  const handleConfirm = () => {
    if (placedShips.length !== SHIPS.length) {
      alert('Please place all ships!');
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
                    onMouseEnter={() => handleCellHover(rowIndex, colIndex)}
                    onMouseLeave={() => setHoverCells([])}
                  >
                    {board[rowIndex][colIndex] > 0 && '🚢'}
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
            <p>💡 Click on the board to place the current ship</p>
            <p>💡 Toggle orientation before placing</p>
          </div>
        </div>
      </div>
    </div>
  );
}

export default ShipPlacement;
