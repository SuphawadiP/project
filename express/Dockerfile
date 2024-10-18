# Use the official Node.js image.
FROM node:20

# Set the working directory
WORKDIR /usr/src/app

# Copy package.json and package-lock.json
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy the rest of your application
COPY . .

# Expose the port the app runs on
EXPOSE 3300

# Command to run your app
CMD ["node", "app.js"]