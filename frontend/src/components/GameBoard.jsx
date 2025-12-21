import './GameBoard.css';

function GameBoard({ board, isOwnBoard, onCellClick, disabled, selectedCell, ships = [] }) {
  const rows = 'ABCDEFGHIJ'.split('');
  const cols = Array.from({ length: 10 }, (_, i) => i);

  // Create a map of cell coordinates to ship symbols
  const getShipSymbol = (row, col) => {
    if (!ships || ships.length === 0) {
      return '🚢';
    }
    
    for (const ship of ships) {
      if (!ship) continue;
      for (let i = 0; i < ship.size; i++) {
        const shipRow = ship.row + (ship.horizontal ? 0 : i);
        const shipCol = ship.col + (ship.horizontal ? i : 0);
        if (shipRow === row && shipCol === col) {
          return ship.symbol || '🚢';
        }
      }
    }
    return '🚢';
  };

  const getCellClass = (row, col) => {
    const value = board[row][col];
    let className = 'cell';

    if (isOwnBoard) {
      if (value === 1) className += ' ship';
      if (value === 2) className += ' hit';
      if (value === 3) className += ' miss';
    } else {
      if (value === 2) className += ' hit';
      if (value === 3) className += ' miss';
    }

    if (selectedCell && selectedCell.row === row && selectedCell.col === col) {
      className += ' selected';
    }

    if (!disabled && !isOwnBoard) {
      className += ' clickable';
    }

    return className;
  };

  return (
    <div className="game-board">
      <div className="board-grid">
        <div className="cell header"></div>
        {cols.map(col => (
          <div key={`col-${col}`} className="cell header">{col}</div>
        ))}

        {rows.map((row, rowIndex) => (
          <>
            <div key={`row-${row}`} className="cell header">{row}</div>
            {cols.map(col => (
              <div
                key={`${row}-${col}`}
                className={getCellClass(rowIndex, col)}
                onClick={() => !disabled && onCellClick(rowIndex, col)}
              >
                {board[rowIndex][col] === 2 && '💥'}
                {board[rowIndex][col] === 3 && '💧'}
                {isOwnBoard && board[rowIndex][col] === 1 && getShipSymbol(rowIndex, col)}
              </div>
            ))}
          </>
        ))}
      </div>

      <div className="board-legend">
        {isOwnBoard && ships && ships.length > 0 && (
          <>
            {ships.map((ship, index) => (
              <div key={index} className="legend-item">
                <span className="legend-symbol">{ship.symbol}</span>
                <span>{ship.name || `Ship ${index + 1}`}</span>
              </div>
            ))}
          </>
        )}
        {(!isOwnBoard || !ships || ships.length === 0) && (
          <div className="legend-item">
            <span className="legend-symbol">🚢</span>
            <span>Ship</span>
          </div>
        )}
        <div className="legend-item">
          <span className="legend-symbol">💥</span>
          <span>Hit</span>
        </div>
        <div className="legend-item">
          <span className="legend-symbol">💧</span>
          <span>Miss</span>
        </div>
      </div>
    </div>
  );
}

export default GameBoard;
