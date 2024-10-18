const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const mongoose = require('mongoose');
const path = require('path'); // Import the path module

// MongoDB connection
mongoose.connect('mongodb://mongodb:27017/sensorDB', {
    useNewUrlParser: true,
    useUnifiedTopology: true,
})
.then(() => console.log("MongoDB connected"))
.catch(err => console.error("MongoDB connection error:", err));

// Create a schema and model for sensor data
const sensorDataSchema = new mongoose.Schema({
    irValue: Number,
    bpm: Number,
    avgBpm: Number,
    createdAt: { type: Date, default: Date.now }
});

const SensorData = mongoose.model('SensorData', sensorDataSchema);

const app = express();
const PORT = 3300; // The port your Arduino is sending data to
app.use(cors()); // Allow cross-origin requests
app.use(bodyParser.json()); // Parse JSON bodies

// Serve the static HTML file
app.use(express.static(path.join(__dirname, 'public'))); // Serve static files from the 'public' directory

// Route to handle sensor data
app.post('/sensor-data', async (req, res) => {
    const { irValue, bpm, avgBpm, apiKey } = req.body;

    // For security purposes, you can validate the API key
    if (apiKey !== '7fRt29bQiQ1RADp3w7ScHtRJ8Tdrp9fQLE0IByFtXuZMtCARpUaPudVwZxn2z1RCf4ufxpRIpkl13tapFYM0afYrg5zzDWeQFGYpYid>        return res.status(403).send('Forbidden: Invalid API key');
    }

    // Log the received data to the console
    console.log(`IR Value: ${irValue}, BPM: ${bpm}, Avg BPM: ${avgBpm}`);

    // Save data to MongoDB
    const sensorData = new SensorData({ irValue, bpm, avgBpm });
    await sensorData.save();

    // Respond back to the Arduino
    res.status(200).send('Data received successfully');
});

// Route to get the latest sensor data
app.get('/sensor-data', async (req, res) => {
    try {
        const latestData = await SensorData.find().sort({ createdAt: -1 }).limit(1); // Get the latest data
        if (latestData.length > 0) {
            res.json(latestData[0]);
        } else {
            res.status(404).send('No data available');
        }
    } catch (err) {
        res.status(500).send('Error retrieving data');
    }
});

// Start the server
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});