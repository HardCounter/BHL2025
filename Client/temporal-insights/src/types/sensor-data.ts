export interface SensorData {
  id: number;
  timestamp: string; // ISO string
  temp_air: number;
  temp_wall: number;
  wilg_pow: number;
  mech: number;
  woda: number;
  dym: number;
  ogien: number;
}

export interface ChartDataPoint {
  timestamp: string;
  value: number | null;
}