import React from 'react';

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

  return (
    <div className="w-full">
      <div className="grid grid-cols-11 gap-0.5 bg-gray-300 p-0.5 rounded-lg">
        <div className="aspect-square bg-gray-600 text-white text-xs font-bold flex items-center justify-center rounded"></div>
        {cols.map(col => (
          <div key={`col-${col}`} className="aspect-square bg-gray-600 text-white text-xs font-bold flex items-center justify-center rounded">
            {col}
          </div>
        ))}

        {rows.map((row, rowIndex) => (
          <React.Fragment key={`row-${row}`}>
            <div className="aspect-square bg-gray-600 text-white text-xs font-bold flex items-center justify-center rounded">
              {row}
            </div>
            {cols.map(col => {
              const cellValue = board[rowIndex][col];
              const isSelected = selectedCell && selectedCell.row === rowIndex && selectedCell.col === col;
              const isShip = isOwnBoard && cellValue === 1;
              const isHit = cellValue === 2;
              const isMiss = cellValue === 3;
              const isClickable = !disabled && !isOwnBoard;

              let cellClasses = "aspect-square bg-white text-lg font-bold flex items-center justify-center transition-all relative";
              
              if (isShip) {
                cellClasses += " bg-green-200";
              } else if (isHit) {
                cellClasses += " bg-red-200";
              } else if (isMiss) {
                cellClasses += " bg-blue-100";
              }
              
              if (isSelected) {
                cellClasses += " ring-2 ring-yellow-400 ring-offset-1";
              }
              
              if (isClickable) {
                cellClasses += " cursor-crosshair hover:bg-blue-50 hover:scale-105";
              }

              return (
                <div
                  key={`${row}-${col}`}
                  className={cellClasses}
                  onClick={() => !disabled && onCellClick(rowIndex, col)}
                >
                  {isHit && '💥'}
                  {isMiss && '💧'}
                  {isShip && getShipSymbol(rowIndex, col)}
                </div>
              );
            })}
          </React.Fragment>
        ))}
      </div>

      {/* Legend */}
      <div className="mt-4 flex flex-wrap gap-4 text-sm">
        {isOwnBoard && ships && ships.length > 0 && (
          <>
            {ships.map((ship, index) => (
              <div key={index} className="flex items-center gap-2">
                <span className="text-lg">{ship.symbol}</span>
                <span className="text-gray-600">{ship.name || `Ship ${index + 1}`}</span>
              </div>
            ))}
          </>
        )}
        {(!isOwnBoard || !ships || ships.length === 0) && (
          <div className="flex items-center gap-2">
            <span className="text-lg">🚢</span>
            <span className="text-gray-600">Ship</span>
          </div>
        )}
        <div className="flex items-center gap-2">
          <span className="text-lg">💥</span>
          <span className="text-gray-600">Hit</span>
        </div>
        <div className="flex items-center gap-2">
          <span className="text-lg">💧</span>
          <span className="text-gray-600">Miss</span>
        </div>
      </div>
    </div>
  );
}

export default GameBoard;
