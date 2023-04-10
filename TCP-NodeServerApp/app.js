const net = require('net');

const server = net.createServer((socket) => {
    console.log('Client connected.');

    // Receive data from the client
    socket.on('data', (data) => {
    try {
      const json = JSON.parse(data.toString());
      console.log('Received:', json);

      // If the JSON structure is incorrect, return 'error'
      if (!json.filePath || typeof json.filePath !== 'string' || !json.username || typeof json.username !== 'string' || !json.isAdmin || typeof json.isAdmin !== 'boolean') {
        socket.write('error');
        return;
      }

      // Send confirmation message to the client
      socket.write('Received successfully');
    } catch (err) {
      console.error('Error parsing JSON:', err);
      socket.write('error');
    }
  });

    socket.on('end', () => {
        console.log('Client disconnected.');
    });
});

server.on('error', (err) => {
    console.error(`Server error: ${err}`);
});

server.listen(8000, () => {
    console.log('Server listening on port 8000');
});

