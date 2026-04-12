# Internet of Things Project Assignment

- **Deadline:** Cuối tháng 04/2026 (dự kiến)
- **Group members:** 2-3 students per group
- **Submission:** PDF report and GitHub link

---

## 1. Project Overview
In this project, students will gain hands-on experience with real-time embedded systems by modifying and extending an existing RTOS-based project. You will work with the project `YoloUNO_PlatformIO - RTOS_Project`, which is implemented on a microcontroller development board.

The mission is to develop a new, complete project with functionalities that differ by at least **30%** from the original code base. The board used in this project will be setup at **301B9** and **812H6**.

**Resources:**
- [Manual Videos](https://www.youtube.com/watch?v=l7AqpsF1ByQ&list=PLyD_mbw_VznMG_tZ-KAGOh6hRPceHuIl5)

---

## 2. Project Tasks and Requirements
Students can propose additional features, but they must be equivalent in complexity to the tasks below. All development must be on the **ESP32 S3** platform using **PlatformIO**.

### Task 1: Single LED Blink with Temperature Conditions
- Redefine LED blinking behavior for at least **3 different temperature conditions**.
- Use **semaphores** for task synchronization.
- Clearly explain condition handling and semaphore logic in the code and report.

### Task 2: NeoPixel LED Control Based on Humidity
- Redefine NeoPixel (RGB LED) patterns for at least **3 humidity levels/colors**.
- Utilize **semaphore synchronization** for updating and displaying colors.
- Map humidity ranges to specific colors clearly.

### Task 3: Temperature and Humidity Monitoring with LCD Display
- Define semaphore conditions based on sensor readings.
- Implement at least **3 display states** (e.g., normal, warning, critical) triggered by semaphores.
- **Remove ALL global variables** by using RTOS semaphores/mechanisms.

### Task 4: Web Server in Access Point Mode
- Redesign the web server interface for better usability.
- Control at least **two devices** (e.g., LED1 and LED2).
- Interface must include at least two buttons and clear labels.

### Task 5: TinyML Deployment & Accuracy Evaluation
- Describe the dataset (collection and labeling).
- Implement and run a TinyML model on the microcontroller.
- Measure accuracy on hardware and provide a performance discussion.

### Task 6: Data Publishing to CoreIOT Cloud Server
- Publish sensor data to [CoreIOT](https://app.coreiot.io/).
- **Mode:** ESP32 S3 must be in **Station (STA) mode**.
- Use the correct authentication token for your device.
- Use the CoreIOT solution template.

---

## 3. Submission Requirements

### Report (PDF)
- Introduction and Objectives.
- Description of all 6 tasks.
- Implementation highlights (semaphore logic, web server design, TinyML workflow, CoreIOT setup).
- Evaluation/Experimental results (especially for TinyML).
- Group discussion and conclusions.
- Names and roles of all members.
- GitHub repository link.

### Code
- Well-commented, original code hosted on **GitHub**.

**Deadline:** Submit via portal trước cuối tháng 04/2026.

---

## 4. Assessment Rubric

| Criteria | Weight |
| :--- | :--- |
| Functionality and Correctness | 60% |
| Report quality & clarity | 25% |
| Code quality & comments | 5% |
| Innovation & Improvements | 5% |
| Group organization & Git Contribution | 5% |

*Bonus points for novel features, creative UI, or advanced TinyML applications.*

---

## 5. Regulations
- 2-3 members per group.
- Equal contribution from all members (must be stated in the report).
- **Plagiarism is strictly prohibited** and will result in disqualification.

---

## 6. Support and Resources
- **Original Project:** [GitHub Repository](https://github.com/nhanksd85/YoloUNO_PlatformIO/tree/RTOS_Project)
- **Technical Support:** Zalo group of the course.

---

## 7. Timeline
- **Project Q&A:** Discuss in Zalo Group.
- **Project Submission:** Cuối tháng 04/2026
