import data from '../data/results.json';
import './style.css';
import {renderReport} from './report';
document.querySelector<HTMLDivElement>('#app')!.innerHTML = renderReport(data);
