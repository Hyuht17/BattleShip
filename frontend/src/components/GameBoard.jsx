import './GameBoard.css';

function GameBoard({ board, isOwnBoard, onCellClick, disabled, selectedCell }) {
  const rows = 'ABCDEFGHIJ'.split('');
  const cols = Array.from({ length: 10 }, (_, i) => i);

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
                {isOwnBoard && board[rowIndex][col] === 1 && '🚢'}
              </div>
            ))}
          </>
        ))}
      </div>

      <div className="board-legend">
        <div className="legend-item">
          <span className="legend-symbol">🚢</span>
          <span>Ship</span>
        </div>
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
