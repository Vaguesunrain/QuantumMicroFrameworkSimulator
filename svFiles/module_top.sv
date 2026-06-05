module module_top (
    input  logic        clk,
    input  logic        rst_n,
    output logic   trigger,
    output logic[31:0]   control_bus//8(control signal):12(first qubit position):12(second qubit position)
    
);
   logic [7:0] counter;
   logic [31:0] raw_instr;
   logic        instr_empty;
   logic        fetch_en;

   instructionFetchUnit u_ifu (
      .clk           (clk),
      .rst_n         (rst_n),
      .next_instr_en (fetch_en),
      .current_instr (raw_instr),
      .stream_end    (instr_empty)
   );

   always_ff @(posedge clk or negedge rst_n) begin
      if (!rst_n) begin
         counter     <= 8'd0;
      end else begin
         counter <= counter + 1;
      end
   end

   always_comb begin
      if (!rst_n) begin
         trigger     = 1'b0;
         control_bus = 32'd0;
         fetch_en = 1'd0;
      end else if (counter > 8'd5 && counter < 8'd13 && !instr_empty) begin
         fetch_en = 1'b1;
         trigger     = 1'b1; 
         control_bus = raw_instr; 
      end else begin  
         trigger     = 1'b0;
         fetch_en = 1'd0;
         control_bus = 32'd0;
      end
end

endmodule
