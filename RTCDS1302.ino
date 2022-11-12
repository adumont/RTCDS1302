#define DATA     16
#define CLK      14
#define CE       10

#define REG_SECONDS   0x80
#define REG_MINUTES   0x82
#define REG_HOURS     0x84
#define REG_DAY       0x86
#define REG_MONTH     0x88
#define REG_DAYNUM    0x8A
#define REG_YEAR      0x8C
#define REG_WP        0x8E
#define REG_CLK_BURST 0xBE

#define READ  0x01
#define WRITE 0x00

uint8_t clock_data[8];
uint8_t s;
char buf[10];

uint8_t dec2bcd(uint8_t dec)
{
    return ((dec / 10 * 16) + (dec % 10));
}

uint8_t bcd2dec(uint8_t bcd)
{
    return ((bcd / 16 * 10) + (bcd % 16));
}

void pulse(uint8_t pin) {
  digitalWrite(pin,HIGH);
  digitalWrite(pin,LOW);
}

void send_data(uint8_t data) {
  pinMode(DATA, OUTPUT);

  for(uint8_t i=0;i<8;i++){
    digitalWrite(DATA, data & 1 );
    pulse(CLK);
    data = data >> 1;
  }
  
}

uint8_t read_data() {
  uint8_t data = 0x00;
  uint8_t b = 0;

  pinMode(DATA, INPUT);

  for(uint8_t i=0;i<8;i++){
    b = digitalRead(DATA);
    data = data | (b<<i);
    pulse(CLK);
  }

  return data;
}

void write_command(uint8_t cmd, uint8_t data) {
  // SCLK must be at a logic 0 when CE is driven to a logic 1 state.
  digitalWrite(CLK, LOW);
  
  digitalWrite(CE, HIGH);
  send_data( WRITE | cmd );
  send_data(data);
  digitalWrite(CE, LOW);
}

uint8_t read_command(uint8_t cmd) {
  uint8_t tmp;
  
  // SCLK must be at a logic 0 when CE is driven to a logic 1 state.
  digitalWrite(CLK, LOW);
  
  digitalWrite(CE, HIGH);
  send_data( READ | cmd );
  tmp = read_data();
  digitalWrite(CE, LOW);

  return tmp;
}

uint8_t read_ram(uint8_t addr) {
  return read_command( 0xC1 + (addr << 1) );
}

uint8_t write_ram(uint8_t addr, uint8_t data) {
  write_command( 0xC0 + (addr << 1), data );
}

void burst_read_clock() {
  uint8_t tmp;
  
  // SCLK must be at a logic 0 when CE is driven to a logic 1 state.
  digitalWrite(CLK, LOW);
  
  digitalWrite(CE, HIGH);
  send_data( REG_CLK_BURST + 1 );   // read register is + 1

  for(uint8_t i = 0; i<8; i++) {
    clock_data[i]  = read_data(); // store the raw BCD + flags data
  }
  
  digitalWrite(CE, LOW);

}

void setup() {
  digitalWrite(CE, LOW);
  digitalWrite(CLK, LOW);
  digitalWrite(DATA, LOW);

  pinMode(CE, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(CLK, OUTPUT);

  Serial.begin(115200);
  while(!Serial);

//  for(uint8_t i = 0; i<31; i++) {
//    Serial.print( i  );
//    Serial.print(": ");
//    Serial.println( read_ram(i) );
//  }
//  
//  for(uint8_t i = 0; i<31; i++) {
//    write_ram(i, i);
//  }

//  write_command(0x80, 0x00);  // set seconds to 0 and start clock

//  write_command(REG_DAY,    0x12);
//  write_command(REG_MONTH,  0x11);
//  write_command(REG_YEAR,   0x22);
//  write_command(REG_DAYNUM, 0x06);
//
//  write_command(REG_HOURS  , 0x14);
//  write_command(REG_MINUTES, 0x30);
//  write_command(REG_SECONDS, 0x00);
}

void loop() {
  // s = bcd2dec( read_command( REG_SECONDS) ); // seconds
  // Serial.println( s );

  burst_read_clock();
//  for(uint8_t i = 0; i<8; i++) {
//    sprintf(buf, "%d: %02x", i, clock_data[i] );
//    Serial.println(buf);
//  }
  sprintf(buf, "%02x/", clock_data[3] ); Serial.print(buf); // Day
  sprintf(buf, "%02x/", clock_data[4] ); Serial.print(buf); // Month
  sprintf(buf, "20%02x ", clock_data[6] ); Serial.print(buf); // Year

  sprintf(buf, "%02x:", clock_data[2] ); Serial.print(buf); // Hours
  sprintf(buf, "%02x:", clock_data[1] ); Serial.print(buf); // Minutes
  sprintf(buf, "%02x" , clock_data[0] ); Serial.print(buf); // Seconds
  
  Serial.println();

  delay(1000);
 
}
