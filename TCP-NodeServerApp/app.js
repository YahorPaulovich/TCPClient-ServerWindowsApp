const net = require('net');

const server = net.createServer((socket) => {
  console.log('Client connected');

  // Receive data from the client
  socket.on('data', (data) => {
    try {
      const json = JSON.parse(data.toString());
      console.log('Received:', json);

      // If the JSON structure is incorrect, return 'error'
      if (!json.filePath || typeof json.filePath !== 'string' || !json.isAdmin || typeof json.isAdmin !== 'boolean') {
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
});

server.on('error', (err) => {
  console.error('Server error:', err);
  throw err;
});

server.listen(10000, () => {
  console.log('Server listening on port 10000');
});